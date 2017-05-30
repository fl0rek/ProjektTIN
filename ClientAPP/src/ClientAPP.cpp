/*
project ClientAPP from MASM
author Michal Citko
date 25.05.2017
*/
#include "Client.h"

#include <cstdio>
#include <thread>
#include <string>
#include <iostream>


using namespace std;

int main(char* argv[], int argc)
{
	if(argv[1] == "1" && argc == 5)
	{
		Client client(argv[2], argv[3], argv[4]);
		thread server_thread(&Client::receiveFromServer, &client);
		thread app_thread(&Client::receiveFromApp, &client);
		app_thread.join();
		server_thread.join();
	}
	else if(argv[1] == "0" && argc == 4)
	{
		Client client(argv[2], argv[3]);
		thread server_thread(&Client::receiveFromServer, &client);
		thread app_thread(&Client::receiveFromApp, &client);
		app_thread.join();
		server_thread.join();
	}
	else
		cout<<"BAD INPUT!\n"<<"first 1 for game mode, 0 for chat mode, then adress, then service, and if game mode then session key"<<endl;
}
