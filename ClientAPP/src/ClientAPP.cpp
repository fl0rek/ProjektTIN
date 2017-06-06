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
	cout<<"iloscgit"<<endl;
	if(!regex_match(argv[1], regex("\\d\\d\?\\d\?\\.\\d\\d\?\\d\?\\.\\d\\d\?\\d\?\\.\\d\\d\?\\d\?")))
		return false;
	cout<<"ipgit"<<endl;
	if(!regex_match(argv[2], regex("\\d\\d\\d\\d\\d\?")))
		return false;

	cout<<"servicetorchegit"<<endl;
	int service = stoi(argv[2]);
	cout<<"servicewartoscd"<<service<<endl;

	if(service <= 1024 || service >= 65535)
		return false;

	cout<<"servicegit"<<endl;
	if(argc == 4)
	{
		uint64_t key = strtoull(argv[3], nullptr, 10);
		
		if(key  == 0ULL || key == ULLONG_MAX)
			return false;
	}

	return true;
}

//TODO add proper argument check
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
