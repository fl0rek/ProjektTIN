#ifndef UTIL_H
#define UTIL_H
#include "Model/player.h"

#include <vector>

std::vector<short> getPlayersIds(const std::vector<Player*> v);
std::string trim(std::string s);
#endif // UTIL_H
