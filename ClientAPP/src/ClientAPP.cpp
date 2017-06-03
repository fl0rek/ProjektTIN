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
//TODO add proper argument check
int main(int argc, char ** argv)
{
	if(argc == 4)
	{
		//TODO add run method
		Client client(argv[1], argv[2], argv[3]);
		thread server_thread(&Client::receiveFromServer, &client);
		thread app_thread(&Client::receiveFromApp, &client);
		app_thread.join();
		server_thread.join();
	}
	else if(argc == 3)
	{
		Client client(argv[1], argv[2]);
		thread server_thread(&Client::receiveFromServer, &client);
		thread app_thread(&Client::receiveFromApp, &client);
		app_thread.join();
		server_thread.join();
	}
	else
		cout<<"BAD INPUT!\n"<<"first adress, then service, and if game mode then session key"<<endl;
}
