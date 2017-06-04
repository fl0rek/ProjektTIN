extern "C" {
#include <server.h>
}

#include <boost/format.hpp>
#include <fstream>
#include <inttypes.h>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <utility>
#include <vector>

#include <tags.h>
#include <fdebug.hh>
#include <Tlv.h>

namespace {
	constexpr char game_image[] = "./GameAPP/Game";

	using namespace tag;

	constexpr unsigned char tags_to_register[] = {internal, game, chat};
}

namespace {
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

int get_client_id(Tlv msg) {
	auto client_id_buffer = msg.getTagData(tag::internal_tags::client_id);
	if(!tag_exists(client_id_buffer))
		return -1;

	return extract_tag<int>(client_id_buffer);
}

bool tag_equal(const unsigned char ltag[4], const unsigned char rtag[4]) {
	return ltag[0] == rtag[0]
		&& ltag[1] == rtag[1]
		&& ltag[2] == rtag[2]
		&& ltag[3] == rtag[3];
}

}

std::mutex libnet_mutex;

namespace libnet_helper {
void sendAuthError(int client_id) {
	unsigned char buffer[sizeof(tag::internal_tags::authentication_error +3)];
	memcpy(buffer, tag::internal_tags::authentication_error, sizeof(tag::internal_tags::authentication_error));
	buffer[sizeof(tag::internal_tags::authentication_error)] = 1;

	{
		std::lock_guard<std::mutex> lock{libnet_mutex};
		libnet_send_to(client_id, tag::internal, sizeof(buffer), buffer);
	}
}
}

class SimpleStatusObserver {
public:
	virtual ~SimpleStatusObserver() {};
	virtual void notify(const unsigned  char tag[4], size_t length, unsigned char *value) = 0;
};

namespace game_helper {

bool stop = false;
std::vector<SimpleStatusObserver*> observers;

void notify_observers(const unsigned  char tag[4], size_t length, unsigned char *value) {
	for(auto obs : observers) {
		obs->notify(tag, length, value);
	}
}

void handle_reader(int read_handle) {
	while(!stop) {
		unsigned char tag[4];
		read(read_handle, tag, 4); //TODO real reading
		unsigned char flipping_flag[1];
		read(read_handle, flipping_flag, 1);
		unsigned char tag_length[1];
		read(read_handle, tag_length, 1);
		unsigned char * body = new unsigned char[tag_length[0]];
		read(read_handle, body, tag_length[0]);

		size_t message_length = sizeof tag + sizeof tag_length + sizeof flipping_flag + tag_length[0];

		// this is fucking retarted
		unsigned char * buffer = new unsigned char[message_length];
		memcpy(&buffer[0], tag, 4);
		memcpy(&buffer[4], tag_length, 1);
		memcpy(&buffer[5], flipping_flag, 1);
		memcpy(&buffer[6], body, tag_length[0]);

		notify_observers(tag, message_length, buffer);
	}
}

}


class Clients : public SimpleStatusObserver {
public:
	class Client {
	public:
		enum State {
			Connected, Chatter, Player
		};
		Client() : name("UNKNOWN"), state(Connected) {}

		template<State S>
		bool is() {
			return state = S;
		}

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

	void request_while_replay_for_client(int client_id) {
		requested_whole_replay.push_back(client_id);
	}

	virtual void notify(const unsigned  char tag[4], size_t length, unsigned char *value) {
		if(tag_equal(tag, tag::game_tags::resync_response)) {
			for(const int client_id : requested_whole_replay) {
				std::lock_guard<std::mutex> lock{libnet_mutex};
				libnet_send_to(client_id, tag::game, length, value);
			}

		}
	}

private:
	int max_id = 0;
	std::vector<Client> clients;

	std::vector<int> requested_whole_replay;
};

Clients cs;

class GameAbstraction {
public:
	virtual ~GameAbstraction() {};
	virtual bool init_ok() const = 0;
	virtual void handle_game_message(const unsigned char* buffer, const size_t length) = 0;
};

class Replay : public GameAbstraction {
public:
	Replay(const std::string &replay_fn) {
		try {
			replay_file.open(replay_fn, std::ifstream::in);
		} catch(std::ifstream::failure e) {
			std::cout << e.what();
			// errors are chaecked later
		}
	}

	void replay_thread_start() {
		replay_thread = std::thread( [this] { replay_main(); } );
	}

	bool init_ok() const {
		return replay_file.is_open() && replay_thread.joinable();
	}

	void handle_game_message(const unsigned char* buffer, const size_t length) {
		log_warn("Unexpected game message received while in replay mode, ignoring");
	}
private:

	void replay_main() {
		while(!stop_thread) {
			size_t header; // only size
			unsigned char body[256];
			replay_file >> header;
			if(!stop_thread) {
				replay_file.read(reinterpret_cast<char*>(body), header);
			}
			sleep(10);
		}
	}

	bool stop_thread = false;
	std::ifstream replay_file;
	std::thread replay_thread;
};

class Game : public SimpleStatusObserver, public GameAbstraction {
public:
	Game(std::string replay_fn) {
		try {
			replay_file.open(replay_fn, std::ifstream::out);
		} catch(std::ifstream::failure e) {
			std::cout << e.what();
			// errors are chaecked later
		}
	}
	~Game() {
		game_process_stop();
	}

	virtual void notify(const unsigned  char tag[4], size_t length, unsigned char *value) {
		libnet_send(tag::game, length, value);
	}

	void game_process_start(char const * game_image) {
		int pid;
		int parent_to_child[2];
		int child_to_parent[2];
		if(pipe(parent_to_child) || pipe(child_to_parent)) {
			log_err("Can't pipe");
			exit(1);
		}

		char * const args[] = {const_cast<char*>(game_image), "2", 0}; // what could go wrong

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
				write_handle = parent_to_child[1];
				read_handle = child_to_parent[0];

				game_observer = std::thread(game_helper::handle_reader, read_handle);
		}
	}

	void game_process_stop() {
		if(game_observer.joinable()) {
			game_helper::stop = true;
			game_observer.join();
		}
		if(game_pid > 0) {
			kill(game_pid, SIGTERM);
			game_pid = -1;
		}
	}

	void handle_game_message(const unsigned char* buffer, const size_t length) {
		log_info("Handling game message [%s:%lu]", buffer, length);

		dump(buffer, length);

		Tlv game_data(buffer, length);
		int client_id = get_client_id(game_data);
		if(client_id < 0)
			return; // ignore msg

		Clients::Client &c = cs.getClient(client_id);
		if(!c.isPlayer()) {
			libnet_helper::sendAuthError(client_id);
			return;
		}

		record_to_replay_file(length, buffer);

		size_t written = 0;
		while(written < length) {
			ssize_t w =  write(write_handle, buffer + written, length - written);
			if(w < 0) {
				perror("game pipe write");
				return;
				//TODO error handling
			}
			written += w;
		}
	}

	void record_to_replay_file(const size_t length, const unsigned char* buffer) {
		replay_file << length;
		replay_file.write(reinterpret_cast<const char*>(buffer), length);
	}

	bool init_ok() const {
		if(!replay_file.is_open()) {
			log_err("Error openging replay file");
			return false;
		}
		if(game_pid < 0) {
			log_err("Forking went wrong");
			return false;
		}
		int status;
		pid_t result = waitpid(game_pid, &status, WNOHANG);
		if(result != 0) {
			log_err("Game excited early");
			return false; // game process not running
		}
		return true;

	}
private:
	int write_handle;
	int read_handle;

	int game_pid;

	std::thread game_observer;

	std::ofstream replay_file;

};


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

	{
		std::lock_guard<std::mutex> lock{libnet_mutex};
		libnet_send(chat, length, buffer);
	}
}

namespace authentication {

std::vector<uint64_t> keys;

void load_player_keys(std::string fn) {
	std::ifstream file;
	file.open(fn, std::ifstream::in);

	std::string key_serialized;
	while (getline(file, key_serialized))
		keys.push_back(std::stoul(key_serialized));
}

Clients::Client::State try_authenticate(uint64_t key) {

	for(const auto k : keys) {
		if(k == key) {
			return Clients::Client::Player;
		}
	}

	return Clients::Client::Connected; // default state
}

}

void handle_internal_message(const unsigned char* buffer, const size_t length) {
	dump(buffer, length);
	Tlv internal_data(buffer, length);
	int client_id = get_client_id(internal_data);

	if(client_id < 0)
		return;

	Clients::Client &c = cs.getClient(client_id);

	if(c.isUnauthenticated()) {
		if(tag_exists(internal_data.getTagData(tag::internal_tags::authentication_code))) {
			uint64_t key = extract_tag<uint64_t>(
					internal_data.getTagData(tag::internal_tags::authentication_code));
			c.authenticate(authentication::try_authenticate(key));
		} else {
			std::string name = reinterpret_cast<char*>(
					&internal_data.getTagData(tag::internal_tags::requested_name)[0]);
			c.authenticate(Clients::Client::Chatter, name);
		}
		if(c.isUnauthenticated()) {
			libnet_helper::sendAuthError(client_id);
			return;
		}
	}
}

void print_usage_and_exit(char* image) {
	std::string usage_string =
"%1% usage: \n\
\t%1% $port $keyfile [ -r $record_file | -p $play_file ] \n\
";
// 0  1     2          3  4              3  4
	std::cout << boost::format(usage_string) % image;
	exit(0);
}

struct {
	enum {
		Play, Replay
	} mode;

	std::string replay_file;
	std::string key_file;
	int port;
} runtime_info;

void parse_args(int argc, char* argv[]) {
	if(argc != 5) {
		print_usage_and_exit(argv[0]);
	}

	std::string mode = argv[3];
	runtime_info.replay_file = argv[4];
	runtime_info.key_file = argv[2];

	if(mode == "-r") { // record mode
		runtime_info.mode = runtime_info.Play;
	} else if(mode == "-p") { // play mode
		runtime_info.mode = runtime_info.Replay;
	} else {
		print_usage_and_exit(argv[0]);
	}

	try {
		runtime_info.port = std::stoi(argv[1]);
	} catch(std::logic_error e) {
		std::cout << e.what() << "\n";
		print_usage_and_exit(argv[0]);
	}
}

int main(int argc, char* argv[]) {
	parse_args(argc, argv);

	authentication::load_player_keys(runtime_info.key_file);

	std::unique_ptr<GameAbstraction> g;

	libnet_init(tags_to_register,
			sizeof(tags_to_register)/sizeof(*tags_to_register));
	log_info1("Starting server thread");
	libnet_thread_start(runtime_info.port); // listening on any interface

	if(runtime_info.mode == runtime_info.Play) {
		Game* gg = new Game(runtime_info.replay_file);
		gg->game_process_start(game_image);
		g = std::unique_ptr<Game>(gg);
	} else {
		g = std::unique_ptr<Replay>(new Replay(runtime_info.replay_file));
	}

	if(!g->init_ok()) {
		log_err("Cannot start game");
		libnet_thread_shutdown();
		exit(-1);
	}


	while(true) {
		libnet_wait_for_new_message();

		ssize_t buffer_len;
		unsigned char buffer[256];
		memset(buffer, 0, sizeof buffer);

		buffer_len = libnet_wait_for_tag(tag::internal, buffer, sizeof(buffer), false);
		if(buffer_len > 0) {
			debug("got %s:%d", buffer, buffer_len);
			handle_internal_message(buffer, buffer_len);
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
			g->handle_game_message(buffer, buffer_len);
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

