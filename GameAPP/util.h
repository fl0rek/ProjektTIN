#ifndef UTIL_H
#define UTIL_H
#include "Model/player.h"

#include <vector>


std::vector<std::string> getPlayersNicks(const std::vector<Player*> v);
std::string trim(std::string s);
#endif // UTIL_H
