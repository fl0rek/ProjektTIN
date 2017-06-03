#pragma once
/*
project ClientAPP from MASM
author Michal Citko
date 25.05.2017
*/

#include <exception>
#include <stdexcept>

class NetworkError : public std::runtime_error
{
	public:
		explicit NetworkError(const std::string& what) :
			std::runtime_error(what) {};
		
		explicit NetworkError(const char* what) :
			std::runtime_error(what) {};

		virtual ~NetworkError() {};
};

class ChildAppError : public std::runtime_error
{
	public:
		explicit ChildAppError(const std::string& what) :
			std::runtime_error(what) {};
		
		explicit ChildAppError(const char* what) :
			std::runtime_error(what) {};

		virtual ~ChildAppError() {};
};
