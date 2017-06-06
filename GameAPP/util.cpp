#include "util.h"

#include <QString>

std::vector<short> getPlayersIds(const std::vector<Player*> v)
{
    std::vector<short> ids;
    for(Player *p : v)
        ids.insert(ids.begin(), p->getId());
    return ids;
}

std::string trim(std::string s)
{
    QString str = QString::fromStdString(s);
    str = str.trimmed();
    return str.toStdString();
}
