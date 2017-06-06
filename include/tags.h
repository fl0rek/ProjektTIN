#pragma once

#ifdef __cplusplus

namespace tag {
	// IMPORTANT:
	// because of quirks in libtlv request type tags have to be not-empty
	// to be recognized (can contain any data)

	constexpr unsigned char internal = 0x11;
	namespace internal_tags {
		// client id, used everywhere
		constexpr unsigned char client_id[] = { 0x11, 0x00, 0x00, 0x01 };
		// auth code
		// valid usages:
		// 	client -> server
		constexpr unsigned char authentication_code[] = { 0x11, 0x00, 0x00, 0x02 };
		// should accompany auth code, otherwise random name given
		// valid usages:
		// 	client -> server
		constexpr unsigned char requested_name[] = { 0x11, 0x00, 0x00, 0x03 };
		// sent in case of auth error or not authenticated client
		// valid usages:
		// 	server -> client
		constexpr unsigned char authentication_error[] = { 0x11, 0x00, 0x00, 0x04 };
	}
	constexpr unsigned char game = 0x12;
	namespace game_tags {
		// request whole game data
		// valid usages:
		// 	game -> client (in case of inconsistency i guess?)
		// 	client -> server // passthrough
		// 	server -> game
		constexpr unsigned char resync_request[] = { 0x12, 0x00, 0x00, 0x01 };
		// whole data response
		// valid usages:
		// 	game -> server
		// 	server -> client // passthrough
		// 	client -> game
		constexpr unsigned char resync_response[] = { 0x12, 0x00, 0x00, 0x02 };
		// move performed by player
		// valid usages:
		// 	game -> player client
		// 	player client -> server
		// 	server -> game
		// 	server -> client
		// 	client -> game
		constexpr unsigned char step[] = { 0x12, 0x00, 0x00, 0x03 };
		// validity of last sent step should contain 1 byte with 1/0
		// valid usages:
		// 	game -> server
		// 	server -> client (if client sends invalid move)
		constexpr unsigned char step_response[] = { 0x12, 0x00, 0x00, 0x04 };
		constexpr unsigned char terminate[] = { 0x15, 0x00, 0x00, 0x04 };
		constexpr unsigned char invalid_step[] = { 0x16, 0x00, 0x00, 0x04 };

		// add user to game
		// valid usages:
		// 	server -> game
		// 	server -> client
		constexpr unsigned char add_client[] = { 0x12, 0x00, 0x00, 0x05 };

		// start game, game should respond with `step` I guess
		// valid usages:
		// 	server -> game
		constexpr unsigned char start_game[] = { 0x12, 0x00, 0x00, 0x06 };
	}
	constexpr unsigned char chat = 0x13;
	namespace chat_tags {
		constexpr unsigned char nick[] = { 0x13, 0x00, 0x00, 0x01 };
		//TODO(Szymon? ktos od chatu anyway)
		constexpr unsigned char message[] = { 0x13, 0x00, 0x00, 0x02 };
	}
}
#else

#define TAG_SIZE 4

// oh god, linking

extern const char tag_internal_client_id[TAG_SIZE];

#endif
