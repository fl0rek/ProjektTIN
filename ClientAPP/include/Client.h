#pragma once

/*
project ClientAPP from MASM
author Michal Citko
date 25.05.2017
*/

#include "Exceptions.h"
#include "libnet/include/client.h"
#include "libnet/common.h"
#include "include/tags.h"

#include <unistd.h>
#include <string.h>

constexpr unsigned char tags_to_register[] = {tag::internal, tag::game, tag::chat};

class Client
{

	public:
		Client(const char * const address, const char * const service, 
				const char * const session_key);
		Client(const char * const address, const char * const service);

		void receiveFromApp() noexcept;
		void receiveFromServer() noexcept;

	private:
		bool startChatApp() noexcept;
		bool startGameApp() noexcept;
		void changeToViewerMode() noexcept;
		bool createPipes() noexcept; 
		bool createGameAppPipes() noexcept;
		bool createChatAppPipes() noexcept;
		bool connectToServer(const  char * const address, 
				const  char * const service) noexcept;
		void requestGameSynchronisation();
		void inline sendToGame(const unsigned char * const data, 
				const ssize_t size) const;
		void inline sendToChat(const unsigned char * const data, 
				const ssize_t size) const;
		bool inline sendToServer(const unsigned char tag, 
			const unsigned char * const data, const ssize_t size) const noexcept;
		void receiveFromGame();
		void receiveFromChat() const;


		const bool kGameMode = 1;
		const bool kChatMode = 0;
		static const ssize_t kReceiveBufferSize = 1000;
		const char kGameApp[5] = "game";
		const char kChatApp[5] = "chat";

		char session_key_[32];
		int app_pid_;
		int pipefd_game_in_[2];
		int pipefd_game_out_[2];
		int pipefd_chat_in_[2];
		int pipefd_chat_out_[2];
		bool mode_;
		unsigned char last_state_[kReceiveBufferSize];
		ssize_t last_state_size_ = 0;
};
