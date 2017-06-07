/*
 * 					HEADER_HEAD
 * author: Mikolaj Florkiewicz
 * 					HEADER_TAIL
 */
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

#include "Replay.hh"
#include "Game.hh"
#include "util.hh"

#include <Exceptions.h>

namespace {
	constexpr char game_image[] = "./GameAPP/Game";

	using namespace tag;

	constexpr unsigned char tags_to_register[] = {internal, game, chat};
}

GameAbstraction *g;

bool server_exiting = false;

bool handle_game_message(const unsigned char* buffer, const size_t length) {
	log_info("Handling game message [%s:%lu]", buffer, length);

	util::dump(buffer, length);
	int client_id = -1;

	try {
		Tlv game_data(buffer, length);
		client_id = util::get_client_id(game_data);
	} catch(TlvException &e) {
		log_err("Received malformed game message");
		return true; // it's not fatal error
	}

	if(client_id < 0)
		return true; // ignore msg

	Clients::Client &c = cs.getClient(client_id);
	if(!c.isPlayer()) {
		libnet_helper::sendAuthError(client_id);
		return true;
	}

	return g->handle_game_message(length, buffer);
}

void handle_chat_message(const unsigned char* buffer, const size_t length) {
	log_info("Handling chat message");

	try {
		Tlv chat_data(buffer, length);
		int client_id = util::get_client_id(chat_data);

		if(client_id < 0)
			return;

		Clients::Client &c = cs.getClient(client_id);

		if(!c.isChatter()) {
			libnet_helper::sendAuthError(client_id);
		}

		const char *name = c.getName().c_str();

		chat_data.add(tag::chat_tags::nick, 0, strlen(name),
			reinterpret_cast<const unsigned char*>(name));
	} catch(TlvException &e) {
		log_err("Malformed chat message");
	}

	{
		std::lock_guard<std::mutex> lock{libnet_mutex};
		if(!libnet_send(chat, length, buffer)) {
			log_warn("Failed to send chat message to some connected chatters");
		}
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
	for(auto it = keys.begin(); it != keys.end(); it++) {
		if(*it == key) {
			using std::swap;
			swap(*it, keys.back());
			keys.pop_back();
			return Clients::Client::Player;
		}
	}

	return Clients::Client::Connected; // default state
}

}

void handle_internal_message(const unsigned char* buffer, const size_t length) {
	util::dump(buffer, length);
	try {
		Tlv internal_data(buffer, length);
	} catch(TlvException &e) {
		log_err("Malformed internal message");
		return;
		// I know this causes buffer to be parsed twice,
		// but since this Tlv class lacks
		// copy constructor this is necessary tradeoff
		// We could wrap this whole function in try
		// catch block but that would be overkill
	}

	Tlv internal_data(buffer, length);
	int client_id = util::get_client_id(internal_data);

	if(client_id < 0)
		return; // malformed message from libnet? ignore

	Clients::Client &c = cs.getClient(client_id);

	if(c.isUnauthenticated()) {
		try {
			if(util::tag_exists(internal_data.getTagData(
					tag::internal_tags::authentication_code))) {

				uint64_t key = util::extract_tag<uint64_t>(
					internal_data.getTagData(
						tag::internal_tags::authentication_code));

				c.authenticate(authentication::try_authenticate(key));
			} else {
				std::string name = reinterpret_cast<char*>(
					&internal_data.getTagData(
						tag::internal_tags::requested_name)[0]);
				c.authenticate(Clients::Client::Chatter, name);
			}
		} catch(TlvException) {
			log_warn("Malformed internal message from %d", client_id);
			// no need for error handling,
			// client will be unathenticated so error message is sent back
		}
		if(c.isUnauthenticated()) {
			libnet_helper::sendAuthError(client_id);
			return;
		}
		if(c.isPlayer()) {
			libnet_helper::sendPlayerHisId(client_id);
			g->add_player(client_id);
			if(authentication::keys.size() == 0) {
				g->start_game();
			}
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

void kill_all_children() {
	signal(SIGQUIT, SIG_IGN);
	kill(-getpid(), SIGQUIT); // kill all children
}

int main(int argc, char* argv[]) {
	atexit(kill_all_children);
	parse_args(argc, argv);

	authentication::load_player_keys(runtime_info.key_file);

	if(!libnet_init(tags_to_register,
			sizeof(tags_to_register)/sizeof(*tags_to_register))) {
		log_err("Could not initialize libnet");
		exit(-10);
	}

	log_info1("Starting server thread");
	libnet_thread_start(runtime_info.port); // listening on any interface

	if(runtime_info.mode == runtime_info.Play) {
		Game* gg = new Game(runtime_info.replay_file);
		game_helper::observers.push_back(gg);
		gg->game_process_start(game_image);
		g = gg;
	} else {
		g = new Replay(runtime_info.replay_file);
	}

	if(!g->init_ok()) {
		log_err("Cannot start game");
		libnet_thread_shutdown();
		exit(-1);
	}

	game_helper::observers.push_back(&cs);

	while(!server_exiting) {
		libnet_wait_for_new_message();

		ssize_t buffer_len;
		unsigned char buffer[256];
		memset(buffer, 0, sizeof buffer);

		buffer_len = libnet_wait_for_tag(tag::internal, buffer, sizeof(buffer), false);
		if(buffer_len > 0) {
			debug("got %s:%ld", buffer, buffer_len);
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
			if(!handle_game_message(buffer, buffer_len)) {
				log_err("Game instance died, bailing out");
				exit(-5);
			}

			continue;
		}

		switch(buffer_len) {
			case -ENOTAG:
				log_err("specified tag not registered");
				exit(-2);
			case -EQUEUE:
				log_err("libnet queue in inconsistent state, should not happen");
				exit(-3);
			case -ESIZE:
				log_err("message too big for buffer, should not occur");
				//should not occour and is not fatal
		}
	}

	libnet_thread_shutdown();
}

