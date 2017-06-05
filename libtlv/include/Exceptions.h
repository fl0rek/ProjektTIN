#pragma once
/*
project libtlv from MASM
author Michal Citko
date 05.06.2017
*/

#include <exception>
#include <stdexcept>

class TlvException : public std::runtime_error
{
	public:
		explicit TlvException(const std::string& what) :
			std::runtime_error(what) {};
		
		explicit TlvException(const char* what) :
			std::runtime_error(what) {};

		virtual ~TlvException() {};
};
