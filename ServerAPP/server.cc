extern "C" {
#include <server.h>
}

#include <fdebug.hh>

namespace {
	constexpr unsigned char game = 0x11;
	constexpr unsigned char chat = 0x12;
}

namespace {
	constexpr unsigned char tags_to_register[] = {game, chat};
}

int main() {
	libnet_init(tags_to_register,
			sizeof(tags_to_register)/sizeof(*tags_to_register));
	log_info1("Starting server thread");
	libnet_thread_start(nullptr); // listening on any interface
}
