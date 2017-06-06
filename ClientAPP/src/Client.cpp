/*
project ClientAPP from MASM
author Michal Citko
date 25.05.2017
*/
#include "Client.h"
#include "Tlv.h"

#include <iostream>
#include <vector>
#include <thread>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

using namespace std;

Client::Client(const char * const address, const char * const service) : mode_(kChatMode)
{
	connectToServer(address, service);
	createPipes();
	setNonblockPipes();
	startChatApp();
}

Client::Client(const char * const address, const char * const service, uint64_t session_key) 
	: mode_(kGameMode), session_key_(session_key)
{
	connectToServer(address, service);
	createPipes();
//	sendSessionKey();
//	receiveAuthentication();
	startGameApp();
	setNonblockPipes();
}

void Client::sendSessionKey() const
{
	ssize_t size = sizeof(uint64_t);
	unsigned char key[size];
	memcpy(const_cast<void *>(static_cast<const void *>(&session_key_)), key, size);

	Tlv buffer;
	buffer.add(tag::internal_tags::authentication_code, 0, size, key);
	vector<unsigned char> full_data = buffer.getAllData();

	if(!libnet_send(tag::internal, full_data.size(), full_data.data()))
		throw NetworkError("cant send session key");
}

void Client::receiveAuthentication() const
{
	
	unsigned char data[kReceiveBufferSize];
	ssize_t size;
	libnet_wait_for_new_message();

	if((size = libnet_wait_for_tag(tag::internal, data, kReceiveBufferSize, true)) > 0)
	{
		Tlv buffer(data, size);
		if((buffer.getTagData(tag::internal_tags::authentication_error).size() != 0))
			throw NetworkError("server did not authenticate me");
		
	}

	switch(size)
	{
		case -ENOTAG:
		case -EQUEUE:
		case -ESIZE:
			throw NetworkError("Libnet reported problem");
	}
}

void Client::run()
{
	std::thread server_thread(&Client::receiveFromServer, this);
	std::thread app_thread(&Client::receiveFromApp, this);
	app_thread.join();
	exit(0);
}

void Client::changeToViewerMode()
{

	startChatApp();
	mode_ = kChatMode;
}

void Client::createPipes()
{
	if(pipe(pipefd_chat_out_) != 0 || pipe(pipefd_chat_in_) != 0 || pipe(pipefd_game_out_) != 0 
			|| pipe(pipefd_game_in_) != 0)
		throw ChildAppError("Cannot create pipes");
}

void Client::setNonblockPipes()
{
	int flags_chat;
	int flags_game;

	if((flags_chat = fcntl(pipefd_chat_in_[0], F_GETFL)) < 0 || 
		(flags_game = fcntl(pipefd_game_in_[0], F_GETFL)) < 0)
		throw ChildAppError("Cannot get flags from pipes");
	
	flags_chat |= O_NONBLOCK;
	flags_game |= O_NONBLOCK;
	
	if((fcntl(pipefd_chat_in_[0], F_SETFL, flags_chat) < 0) 
		|| (fcntl(pipefd_chat_in_[0], F_SETFL, flags_game) < 0))
		throw ChildAppError("Cannot set flags for pipes");
}

void Client::createGameAppPipes()
{
	if(dup2(pipefd_game_out_[0], STDIN_FILENO) < 0 || dup2(pipefd_game_in_[1], STDOUT_FILENO) < 0 
			|| dup2(pipefd_game_in_[1], STDERR_FILENO) < 0 || close(pipefd_game_in_[0]) < 0
			|| close(pipefd_game_in_[1]) < 0 || close(pipefd_game_out_[0]) < 0 
			|| close(pipefd_game_out_[1]) < 0)
		throw ChildAppError("Cannot duplicate pipes");
}

void Client::createChatAppPipes()
{

	if(dup2(pipefd_chat_out_[0], STDIN_FILENO) < 0 || dup2(pipefd_chat_in_[1], STDOUT_FILENO) < 0 
			|| dup2(pipefd_chat_in_[1], STDERR_FILENO) < 0 || close(pipefd_chat_in_[0]) < 0 
			|| close(pipefd_chat_in_[1]) < 0 || close(pipefd_chat_out_[0]) < 0 
			|| close(pipefd_chat_out_[1]))
		throw ChildAppError("Cannot duplicate pipes");
}

void Client::startChatApp()
{
	chat_app_pid_ = fork();
	
	if(chat_app_pid_ == 0)
	{
		createChatAppPipes();

		if(execv(kChatApp, kChatAppParams) == -1)
		{
			throw ChildAppError("Cannot execute chat");
		}
	}	
	else if(chat_app_pid_ == -1)
		throw ChildAppError("Cannot start chat app");

	if(close(pipefd_chat_in_[1]) != 0 || close(pipefd_chat_out_[0]) != 0)
		throw ChildAppError("Cannot close not used pipe ends");
}

void Client::startGameApp()
{
	game_app_pid_ = fork();
	
	if(game_app_pid_ == 0)
	{
		createGameAppPipes();

		if(execv(kGameApp, kGameAppParams) == -1)
			throw ChildAppError("Cannot execute game");
	}	
	else if(game_app_pid_ == -1)
		throw ChildAppError("Cannot start game app");

	if(close(pipefd_game_in_[1]) != 0 || close(pipefd_game_out_[0]) != 0)
		throw ChildAppError("Cannot close not used pipe ends");
}

void Client::connectToServer(const char * const address, const char * const service)
{

	if(!libnet_init(tags_to_register, sizeof(tags_to_register) / sizeof(*tags_to_register)) 
			|| !libnet_thread_start(address, service))
		throw NetworkError("Cannot connect to server");
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

bool inline Client::sendToServer(const unsigned char tag, const unsigned char * const data
	, const ssize_t size) const noexcept
{
	//TODO
	Tlv buffer;// get rid of this Tlv, chat and game should use it
	buffer.add(0x88, 0 , size, data);
	std::vector<unsigned char> full_buffer = buffer.getAllData();
	return libnet_send(tag, full_buffer.size(), full_buffer.data());
}

void Client::receiveFromApp() noexcept
{
	bool end_flag = kClientRun;

	while(1)
	{
		if(end_flag == kClientEnd)
		{
			return;
		}

		if(mode_ == kGameMode)
			receiveFromGame(&end_flag);
		else
			receiveFromChat(&end_flag);
	}
}

void Client::receiveFromGame(bool * const end_flag)
{
	if(waitpid(game_app_pid_, nullptr, WNOHANG) == 0)
	{
		unsigned char data[kReceiveBufferSize];
		ssize_t size = read(pipefd_game_in_[0], data, kReceiveBufferSize);

		if(size < 0 && errno != EAGAIN)
			throw ChildAppError("Problem with pipe, cannot read from game");
//TODO
//	if(endgame)
//		changeToViewerMode();
//	else
		if(size > 0)
			sendToServer(tag::game, data, size);
	}
	else
		*end_flag = kClientEnd;
}

void Client::receiveFromChat(bool * const end_flag)
{
	if(waitpid(chat_app_pid_, NULL, WNOHANG) == 0)
	{
		unsigned char data[kReceiveBufferSize];
		ssize_t size = read(pipefd_chat_in_[0], data, kReceiveBufferSize);

		if(size < 0 && errno != EAGAIN)
			throw ChildAppError("Problem with pipe, cannot read from chat");

		if(size > 0)
			sendToServer(tag::chat, data, size);
	}
	else
		*end_flag = kClientEnd;
}


void Client::receiveFromServer() noexcept
{
	unsigned char data[kReceiveBufferSize];
	ssize_t size;

	while(1)
	{
		libnet_wait_for_new_message();

		if((size = libnet_wait_for_tag(tag::game, data, kReceiveBufferSize, false)) > 0) 
			sendToGame(data, size);
		else if((size = libnet_wait_for_tag(tag::chat, data, kReceiveBufferSize, false)) > 0)
			sendToChat(data + 6, size - 17); //TODO change this, Tlv should bu used by chat to perform this operation
		else if((size = libnet_wait_for_tag(tag::internal, data, kReceiveBufferSize, false)) > 0)
		{
			;
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
