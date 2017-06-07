/*
project ClientAPP from MASM
author Michal Citko
date 25.05.2017
*/
#include "Client.h"

#include <cstdio>
#include <string>
#include <iostream>
#include <regex>
#include <climits>

using namespace std;

bool checkArgs(int argc, char ** argv)
{
	if(argc != 3 && argc != 4)
		return false;

	if(!regex_match(argv[1], regex("\\d\\d\?\\d\?\\.\\d\\d\?\\d\?\\.\\d\\d\?\\d\?\\.\\d\\d\?\\d\?")))
		return false;

	if(!regex_match(argv[2], regex("\\d\\d\\d\\d\\d\?")))
		return false;


	int service = stoi(argv[2]);


	if(service <= 1024 || service >= 65535)
		return false;


	if(argc == 4)
	{
		uint64_t key = strtoull(argv[3], nullptr, 10);
		
		if(key  == 0ULL || key == ULLONG_MAX)
			return false;
	}

	return true;
}

int main(int argc, char ** argv)
{
	if(!checkArgs(argc, argv))
	{
		cout<<"BAD INPUT!\n"<<"first adress, then service, and if game mode then session key"<<endl;
		return 0;
	}

	try
	{
		if(argc == 4)
		{
			Client client(argv[1], argv[2], strtoull(argv[3], nullptr, 10));
			client.run();
		}
		else if(argc == 3)
		{
			Client client(argv[1], argv[2]);
			client.run();
		}
	}
	catch(runtime_error &e)
	{
		cout<<e.what()<<endl;
		return 0;
	}
}
