extern "C" {
#include <server.h>
}

#include <fdebug.hh>
#include <utility>

namespace {
	namespace tag {
		constexpr unsigned char internal = 0x11;
		constexpr unsigned char game = 0x12;
		constexpr unsigned char chat = 0x13;
	}

	constexpr char game_image[] = "./game";

	using namespace tag;

	constexpr unsigned char tags_to_register[] = {internal, game, chat};
}


std::pair<int, int> game_handle;

void handle_chat_message(const unsigned char* buffer, const size_t length) {
	log_info("Handling chat message");
	libnet_send(chat, length, buffer);
}

void handle_game_message(const unsigned char* buffer, const size_t length) {
	log_info("Handling game message [%s:%lu]", buffer, length);
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
}

std::pair<int, int> game_process_start(char const * game_image) {
	int pid;
	int parent_to_child[2];
	int child_to_parent[2];
	if(pipe(parent_to_child) || pipe(child_to_parent)) {
		log_err("Can't pipe");
		exit(1);
	}

	char * const args[] = {const_cast<char*>(game_image), 0};

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
