/*
 * 					HEADER_HEAD
 * author: Mikolaj Florkiewicz
 * 					HEADER_TAIL
 */
#pragma once

#include "GameAbstraction.hh"
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

	bool handle_game_message(const size_t length, const unsigned char* buffer) {
		log_warn("Unexpected game message received while in replay mode, ignoring");
		return true;
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
