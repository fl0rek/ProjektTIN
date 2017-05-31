#include "card.h"

bool compare(Card c1, Card c2)
{
    if(c1.first != c2.first)
        return false;
    if(c1.second != c2.second)
        return false;
    return true;
}
