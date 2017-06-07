/*
 * 					HEADER_HEAD
 * author: Mikolaj Florkiewicz
 * 					HEADER_TAIL
 */
#pragma once

#include "Replay.hh"
#include "Clients.hh"
#include "SimpleStatusObserver.hh"
#include "util.hh"

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
		if(util::tag_equal(tag, tag::game_tags::step)) {
			if(!libnet_send(tag::game, length, value)) {
				log_warn("Failed notifying clients, they'll hopefully catch up on next update");
			}
		}
	}

	void game_process_start(char const * game_image) {
		int pid;
		int parent_to_child[2];
		int child_to_parent[2];
		if(pipe(parent_to_child) || pipe(child_to_parent)) {
			log_err("Can't pipe");
			exit(1);
		}

		char const * game_param = "2";

		char * const args[] = {const_cast<char*>(game_image), const_cast<char*>(game_param), 0}; // we're execing in the moment so I guess everything is ok?

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

	bool handle_game_message(const size_t length, const unsigned char* buffer) {
		record_to_replay_file(length, buffer);

		Tlv game_data(buffer, length);
		if(game_data.isTagPresent(tag::game_tags::resync_request)) {

			int client_id = util::get_client_id(game_data);

			if(client_id < 0)
				return true;

			cs.request_whole_replay_for_client(client_id);
		}

		size_t written = 0;
		while(written < length) {
			ssize_t w =  write(write_handle, buffer + written, length - written);
			if(w < 0) {
				perror("game pipe write, game presumably dead");
				return false;
			}
			written += w;
		}
		return true;
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

	void add_player(int client_id) {
		Tlv buffer;
		buffer.add(tag::game_tags::add_client, 0, 1, 0);
		buffer.add(tag::internal_tags::client_id, 0, sizeof(client_id), reinterpret_cast<unsigned char*>(&client_id));
		std::vector<unsigned char> data = buffer.getAllData();

		size_t written = 0;
		while(written < data.size()) {
			ssize_t w =  write(write_handle, &data[0] + written, data.size() - written);
			written += w;
		}
	}

	void start_game() {
		Tlv buffer;
		buffer.add(tag::game_tags::start_game, 0, 1, 0);
		std::vector<unsigned char> data = buffer.getAllData();

		size_t written = 0;
		while(written < data.size()) {
			ssize_t w =  write(write_handle, &data[0] + written, data.size() - written);
			written += w;
		}
	}
private:
	int write_handle;
	int read_handle;

	int game_pid;

	std::thread game_observer;

	std::ofstream replay_file;

};





