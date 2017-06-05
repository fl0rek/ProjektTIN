/*
 * 					HEADER_HEAD
 * author: Mikolaj Florkiewicz
 * 					HEADER_TAIL
 */
#pragma once

class GameAbstraction {
public:
	virtual ~GameAbstraction() {};
	virtual bool init_ok() const = 0;
	virtual bool handle_game_message(const size_t length, const unsigned char* buffer) = 0;
};

