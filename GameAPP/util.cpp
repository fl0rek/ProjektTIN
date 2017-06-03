#include "util.h"

#include <QString>

std::vector<std::string> getPlayersNicks(const std::vector<Player*> v)
{
    std::vector<std::string> nicks;
    for(Player *p : v)
        nicks.insert(nicks.begin(), p->getNick());
    return nicks;
}

std::string trim(std::string s)
{
    QString str = QString::fromStdString(s);
    str = str.trimmed();
    return str.toStdString();
}
