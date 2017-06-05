#pragma once

class SimpleStatusObserver {
public:
	virtual ~SimpleStatusObserver() {};
	virtual void notify(const unsigned  char tag[4], size_t length, unsigned char *value) = 0;
};
