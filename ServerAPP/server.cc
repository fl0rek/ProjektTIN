extern "C" {
#include <server.h>
}

#include <inttypes.h>
#include <iomanip>
#include <iostream>
#include <vector>
#include <utility>

#include <tags.h>
#include <fdebug.hh>
#include <Tlv.h>

namespace {
	constexpr char game_image[] = "./game";

	using namespace tag;

	constexpr unsigned char tags_to_register[] = {internal, game, chat};
}

namespace libnet_helper {
void sendAuthError(int client_id) {
	unsigned char buffer[sizeof(tag::internal_tags::authentication_error +3)];
	memcpy(buffer, tag::internal_tags::authentication_error, sizeof(tag::internal_tags::authentication_error));
	buffer[sizeof(tag::internal_tags::authentication_error)] = 1;

	libnet_send_to(client_id, tag::internal, sizeof(buffer), buffer);
}
}

class Clients {
public:
	class Client {
	public:
		enum State {
			Connected, Chatter, Player
		};
		Client() : name("UNKNOWN"), state(Connected) {}

		bool isPlayer() const {
			return state == Player;
		}
		bool isChatter() const {
			return state == Chatter;
		}

		bool isUnauthenticated() const {
			return state == Connected;
		}

		void authenticate(const State s, const std::string &name = "") {
			this->state = s;
			if(!name.empty())
				this->name = name;
		}

		std::string getName() const {
			return name;
		}
	private:
		std::string name;
		State state;
	};

	Client& getClient(int id) {
		if(id > max_id) {
			clients.resize(id+1);
			max_id = id;
		}
		return clients[id];
	}

private:
	int max_id = 0;
	std::vector<Client> clients;
};

Clients cs;

void dump(const void *mem, size_t n) {
	const unsigned char *p = reinterpret_cast<const unsigned char*>(mem);
	std::cout << std::endl << "====" << std::endl;
	for(size_t i = 0; i < n; i++) {
		std::cout << std::setw(2) << std::setfill('0') << std::hex << int(p[i]) << " ";
	}
	std::cout << std::endl << "====" << std::endl;
}

class TagNotFoundException : public std::runtime_error {
	using runtime_error::runtime_error;
};

bool tag_exists(std::vector<unsigned char> buffer) {
	return !!buffer.size();
}

template<typename T>
T extract_tag(std::vector<unsigned char> buffer, size_t offset = 0) {
	if(!tag_exists(buffer))
		throw new TagNotFoundException("");
	T ret;
	memcpy(&ret, &buffer[offset], sizeof(T));
	return ret;
}

std::pair<int, int> game_handle;

int get_client_id(Tlv msg) {
	auto client_id_buffer = msg.getTagData(tag::internal_tags::client_id);
	if(!tag_exists(client_id_buffer))
		return -1;

	return extract_tag<int>(client_id_buffer);
}

void handle_chat_message(const unsigned char* buffer, const size_t length) {
	log_info("Handling chat message");

	Tlv chat_data(buffer, length);
	int client_id = get_client_id(chat_data);

	if(client_id < 0)
		return;

	Clients::Client &c = cs.getClient(client_id);

	if(!c.isChatter()) {
		libnet_helper::sendAuthError(client_id);
	}

	const char *name = c.getName().c_str();

	chat_data.add(tag::chat_tags::nick, 0, strlen(name), reinterpret_cast<const unsigned char*>(name));

	libnet_send(chat, length, buffer);
}

void handle_game_message(const unsigned char* buffer, const size_t length) {
	log_info("Handling game message [%s:%lu]", buffer, length);

	dump(buffer, length);

	Tlv game_data(buffer, length);
	int client_id = get_client_id(game_data);
	if(client_id < 0)
		return; // ignore

	Clients::Client &c = cs.getClient(client_id);
	if(!c.isPlayer()) {
		libnet_helper::sendAuthError(client_id);
		return;
	}

	// ONE TAG AT A TIME
	size_t written = 0;
	while(written < length) {
		ssize_t w =  write(game_handle.first, buffer + written, length - written);
		if(w < 0) {
			perror("game pipe write");
			return;
			//error handling
		}
		written += w;
	}

	{
		unsigned char tag[4];
		read(game_handle.second, tag, 4); //TODO real reading
		unsigned char tag_length[1];
		read(game_handle.second, tag_length, 1);
		unsigned char flipping_flag[1];
		read(game_handle.second, flipping_flag, 1);
		unsigned char * body = new unsigned char[tag_length[0]];
		read(game_handle.second, body, tag_length[0]);

		size_t message_length = sizeof tag + sizeof tag_length + sizeof flipping_flag + tag_length[0];

		// this is fucking retarted
		unsigned char * buffer = new unsigned char[message_length];
		memcpy(&buffer[0], tag, 4);
		memcpy(&buffer[4], tag_length, 1);
		memcpy(&buffer[5], flipping_flag, 1);
		memcpy(&buffer[6], body, tag_length[0]);

		libnet_send(tag::game, message_length, buffer);
	}

}

uint64_t chatter_key = 0;
uint64_t player_key = 1;

Clients::Client::State try_authenticate(uint64_t key) {
	// TODO xd
	if(key == chatter_key)
		return Clients::Client::Chatter;
	if(key == player_key)
		return Clients::Client::Player;

	return Clients::Client::Connected; // default state
}

void handle_internal_message(const unsigned char* buffer, const size_t length) {
	Tlv internal_data(buffer, length);
	int client_id = get_client_id(internal_data);

	if(client_id < 0)
		return;

	Clients::Client &c = cs.getClient(client_id);

	if(c.isUnauthenticated() &&
		tag_exists(internal_data.getTagData(tag::internal_tags::authentication_code))) {
		uint64_t key = extract_tag<uint64_t>(
				internal_data.getTagData(tag::internal_tags::authentication_code));
		std::string name = reinterpret_cast<char*>(
				&internal_data.getTagData(tag::internal_tags::requested_name)[0]);
		c.authenticate(try_authenticate(key), name);
		if(c.isUnauthenticated()) {
			libnet_helper::sendAuthError(client_id);
			return;
		}
	}
}

std::pair<int, int> game_process_start(char const * game_image) {
	int pid;
	int parent_to_child[2];
	int child_to_parent[2];
	if(pipe(parent_to_child) || pipe(child_to_parent)) {
		log_err("Can't pipe");
		exit(1);
	}

	char * const args[] = {const_cast<char*>(game_image), 0}; // what could go wrong

	switch(pid = fork()) {
		case -1:
			perror("Can't fork");
			exit(1);
		case 0: //child
			close(1); //stdout
			dup(child_to_parent[1]);

			close(0); //stdin
			dup(parent_to_child[0]);

			close(parent_to_child[1]);
			close(child_to_parent[0]);
			execv(game_image, args);
			perror("Can't exec");
			exit(1);
		default:
			close(child_to_parent[1]);
			close(parent_to_child[0]);
			return std::make_pair(parent_to_child[1], child_to_parent[0]);
	}
}

int main() {
	libnet_init(tags_to_register,
			sizeof(tags_to_register)/sizeof(*tags_to_register));
	log_info1("Starting server thread");
	libnet_thread_start(nullptr); // listening on any interface

	game_handle = game_process_start(game_image);

	while(true) {
		libnet_wait_for_new_message();

		ssize_t buffer_len;
		unsigned char buffer[256];
		memset(buffer, 0, sizeof buffer);

		buffer_len = libnet_wait_for_tag(tag::internal, buffer, sizeof(buffer), false);
		if(buffer_len > 0) {
			debug("got %s", buffer);
			continue;
		}

		buffer_len = libnet_wait_for_tag(tag::chat, buffer, sizeof(buffer), false);
		if(buffer_len > 0) {
			handle_chat_message(buffer, buffer_len);
			continue;
		}

		buffer_len = libnet_wait_for_tag(tag::game, buffer, sizeof(buffer), false);
		if(buffer_len > 0) {
			debug("got %s", buffer);
			handle_game_message(buffer, buffer_len);
			continue;
		}

		switch(buffer_len) {
			case -ENOTAG:
			case -EQUEUE:
			case -ESIZE:
				// lol
				break;
		}


	}

}
