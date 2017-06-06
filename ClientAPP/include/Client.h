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
				uint64_t session_key);
		/**
		 * @brief
		 *		constructs Client object in viewer mode
		 *	@param address
		 *		server address
		 *	@param service
		 *		server port
		*/
		Client(const char * const address, const char * const service);

		/**
		 * @brief
		 *		client starts receiving messages from server and from app	
		*/
		void run();

	private:
		void receiveFromApp() noexcept;
		void receiveFromServer() noexcept;
		void startChatApp();
		void startGameApp();
		void changeToViewerMode();
		void createPipes();
		void createGameAppPipes();
		void createChatAppPipes(); 
		void setNonblockPipes();
		void connectToServer(const  char * const address, 
				const  char * const service);
		void inline sendToGame(const unsigned char * const data,
				const ssize_t size) const;
		void inline sendToChat(const unsigned char * const data,
				const ssize_t size) const;
		bool inline sendToServer(const unsigned char tag,
			const unsigned char * const data, const ssize_t size) const noexcept;
		void receiveFromGame(bool * const end_flag);
		void receiveFromChat(bool * const end_flag);
		void sendSessionKey() const;
		void receiveAuthentication() const;


		const bool kGameMode = 1;
		const bool kChatMode = 0;
		static const ssize_t kReceiveBufferSize = 1000;
		char kGameApp[16] = "GameAPP/Game";
		char kChatApp[19] = "ChatAPP/chatAPP";
		char * const kChatAppParams[2] = {kChatApp, nullptr};
		char * const kGameSpectatorAppParams[3] = {kGameApp, "0", 0};
		char * const kGamePlayerAppParams[3] = {kGameApp, "1", 0};
		const bool kClientRun = true;
		const bool kClientEnd = false;

		uint64_t session_key_;
		int chat_app_pid_;
		int game_app_pid_;
		//TODO change them a little to stop using 0 or 1  
		int pipefd_game_in_[2];
		int pipefd_game_out_[2];
		int pipefd_chat_in_[2];
		int pipefd_chat_out_[2];
		bool mode_;
};
