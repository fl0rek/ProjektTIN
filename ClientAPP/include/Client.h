#pragma once

/*
project ClientAPP from MASM
author Michal Citko
date 25.05.2017
*/

extern "C" {
#include <client.h>
}

#include "Exceptions.h"
#include "libnet/common.h"
#include "include/tags.h"
#include <Tlv.h>

#include <unistd.h>
#include <string.h>

constexpr unsigned char tags_to_register[] = {tag::internal, tag::game, tag::chat};

class Client
{

	public:
		/**
		 * @brief
		 *		constructs Client object in game mode
		 *	@param address
		 *		server address
		 *	@param service
		 *		server port
		 *	@param session_key
		 *		key to authorize gamer
		*/
		Client(const char * const address, const char * const service,
				const char * const session_key);
		/**
		 * @brief
		 *		constructs Client object in viewer mode
		 *	@param address
		 *		server address
		 *	@param service
		 *		server port
		*/
		Client(const char * const address, const char * const service);

		//TODO make them private, add public run method
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
		const char kGameApp[16] = "GameAPP/Game";
		const char kChatApp[19] = "ChatAPP/chatAPP";

		char session_key_[32];
		int app_pid_;
		//TODO change them a little to stop using 0 or 1
		int pipefd_game_in_[2];
		int pipefd_game_out_[2];
		int pipefd_chat_in_[2];
		int pipefd_chat_out_[2];
		bool mode_;
		unsigned char last_state_[kReceiveBufferSize];
		ssize_t last_state_size_ = 0;
};
