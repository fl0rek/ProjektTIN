/*
project ClientAPP from MASM
author Michal Citko
date 25.05.2017
*/
#include "Client.h"
#include <iostream>
#include <vector>

Client::Client(const char * const address, const char * const service) : mode_(kChatMode)
{
	if(!connectToServer(address, service))
		throw NetworkError("Cannot connect to server");

	if(!createPipes())
		throw ChildAppError("Cannot create pipes");

	if(!startChatApp())
		throw ChildAppError("Cannot start chat app");

	if(!startGameApp())
		throw ChildAppError("Cannot start game app");

}

Client::Client(const char * const address, const char * const service, const char * const session_key) 
	: mode_(kGameMode)
{
	memcpy(session_key_, session_key, sizeof(session_key));

	if(!connectToServer(address, service))
		throw NetworkError("Cannot connect to server");

	if(!createPipes())
		throw ChildAppError("Cannot create pipes");
	
	if(!startGameApp())
		throw ChildAppError("Cannot start game app");
}

void Client::changeToViewerMode() noexcept
{

	if(!startChatApp())
		throw ChildAppError("Cannot start chat app after the game app");

	mode_ = kChatMode;
}

bool Client::createPipes() noexcept
{
	if(pipe(pipefd_chat_out_) != 0 || pipe(pipefd_chat_in_) != 0 || pipe(pipefd_game_out_) != 0 
			|| pipe(pipefd_game_in_) != 0)
	{
		return false;
	}
	
	return true;
}

bool Client::createGameAppPipes() noexcept
{
	if(dup2(pipefd_game_out_[0], STDIN_FILENO) != 0 || dup2(pipefd_game_in_[1], STDOUT_FILENO) != 0 
			|| dup2(pipefd_game_in_[1], STDERR_FILENO) != 0 || close(pipefd_game_in_[0]) != 0
			|| close(pipefd_game_in_[1]) != 0 || close(pipefd_game_out_[0]) != 0 
			|| close(pipefd_game_out_[1]) != 0)
	{
		return false;
	}


	return true;
}

bool Client::createChatAppPipes() noexcept
{

	if(dup2(pipefd_chat_out_[0], STDIN_FILENO) != 0 || dup2(pipefd_chat_in_[1], STDOUT_FILENO) != 0 
			|| dup2(pipefd_chat_in_[1], STDERR_FILENO) != 0 || close(pipefd_chat_in_[0]) != 0 
			|| close(pipefd_chat_in_[1]) != 0 || close(pipefd_chat_out_[0]) != 0 
			|| close(pipefd_chat_out_[1]))
	{
		return false;
	}


	return true;
}

bool Client::startChatApp() noexcept
{
	app_pid_ = fork();
	
	if(app_pid_ == 0)
	{
		if(createChatAppPipes() != 0)
		{
			throw ChildAppError("Cannot duplicate pipes");
		}

		if(execl(kChatApp, "", (char*) NULL) == -1)
		{
			throw ChildAppError("Cannot execute chat");
		}
	}	
	else if(app_pid_ == -1)
	{
		return false;
	}

	if(close(pipefd_chat_in_[1]) != 0 || close(pipefd_chat_out_[0]) != 0)
		return false;
	
	return true;
}

bool Client::startGameApp() noexcept
{
	app_pid_ = fork();
	
	if(app_pid_ == 0)
	{
		if(createGameAppPipes() != 0)
			throw ChildAppError("Cannot duplicate pipes");

		if(execl(kGameApp, "", (char*) NULL) == -1)
			throw ChildAppError("Cannot execute game");
	}	
	else if(app_pid_ == -1)
	{
		return false;
	}

	if(close(pipefd_game_in_[1]) != 0 || close(pipefd_game_out_[0]) != 0)
		return false;

	return true;
}

bool Client::connectToServer(const char * const address, const char * const service) noexcept
{

	if(!libnet_init(tags_to_register, sizeof(tags_to_register) / sizeof(*tags_to_register)) 
			|| !libnet_thread_start(address, service))
		return false;

	return true;
}

void Client::requestGameSynchronisation()
{
	;
	//TODO
//	while(!sendToServer(last_state_, last_state_size_));
}

void inline Client::sendToGame(const unsigned char * const data, const ssize_t size) const
{
	ssize_t write_size = write(pipefd_game_out_[1], data, size);

	if(write_size < 0)
		throw ChildAppError("Problem with pipe, cannot write to game");
}

void inline Client::sendToChat(const unsigned char * const data, const ssize_t size) const
{
	ssize_t write_size = write(pipefd_chat_out_[1], data, size);

	if(write_size < 0)
		throw ChildAppError("Problem with pipe, cannot write to chat");
}

bool inline Client::sendToServer(const unsigned char tag, const unsigned char * const data, const ssize_t size) const noexcept
{
	//TODO
	Tlv buffer;// get rid of this Tlv, chat and game should use it
	buffer.add(0x88, 0 , size, data);
	std::vector<unsigned char> full_buffer = buffer.getAllData();
	return libnet_send(tag, full_buffer.size(), full_buffer.data());
}

void Client::receiveFromApp() noexcept
{
	while(1)
	{
		if(mode_ == kGameMode)
			receiveFromGame();
		else
			receiveFromChat();
	}
}

void Client::receiveFromGame()
{
	unsigned char data[kReceiveBufferSize];
	ssize_t size = read(pipefd_game_in_[0], data, kReceiveBufferSize);

	if(size < 0)
		throw ChildAppError("Problem with pipe, cannot read from game");
//TODO
//	if(endgame)
//		changeToViewerMode();
//	else
		sendToServer(tag::game, data, size);
}

void Client::receiveFromChat() const
{
	unsigned char data[kReceiveBufferSize];
	ssize_t size = read(pipefd_chat_in_[0], data, kReceiveBufferSize);

	if(size < 0)
		throw ChildAppError("Problem with pipe, cannot read from chat");

	while(!sendToServer(tag::chat, data, size)); //TODO maybe not in while
}


void Client::receiveFromServer() noexcept
{
	unsigned char data[kReceiveBufferSize];
	ssize_t size;

	while(1)
	{
		libnet_wait_for_new_message();

		if((size = libnet_wait_for_tag(tag::game, data, kReceiveBufferSize, false)) > 0) // with false doesnt block
		{
			sendToGame(data, size);
			memcpy(last_state_, data, sizeof(unsigned char) * size);
			last_state_size_ = size;
		}
		else if((size = libnet_wait_for_tag(tag::chat, data, kReceiveBufferSize, false)) > 0)
		{
			//TODO change this, Tlv should bu used by chat to perform this operation
			sendToChat(data + 6, size - 17);
		}
		else if((size = libnet_wait_for_tag(tag::internal, data, kReceiveBufferSize, false)) > 0)
		{
			//TODO
			std::cout<<"interna";
		}

		switch(size)
		{
			case -ENOTAG:
			case -EQUEUE:
			case -ESIZE:
				throw NetworkError("Libnet reported problem");
		}
	}
}
