#include "player.h"

#include <string>
#include <algorithm>


Player::Player()
{
    
}


Player::Player(std::string nick)
{
    this->nick = nick;
    lastCard = std::make_pair(NO_RANK, NO_SUIT);
}


Player::Player(std::string nick, std::vector<Card> cards) : nick(nick), cards{ std::move(cards) }{}


void Player::giveCard(Card c)
{
    cards.push_back(c);
}


void Player::removeCard(Card c)
{
    cards.erase(std::remove(cards.begin(), cards.end(), c), cards.end());
}


bool Player::hasCard(Card card)
{
    for(Card c : cards)
        if(c == card)
            return true;
    return false;
}


bool Player::hasThree()
{
    unsigned i = 0;
    for (Card c : cards)
    {
        for(Card c2 : cards)
        {
            if(c.first == c2.first)
                i++;
            if(i >= 3)
                return true;
        }
        i = 0;
    }

    for (Card c : cards)
    {
        for(Card c2 : cards)
        {
            if(c.second == c2.second)
                i++;
            if(i >= 3)
                return true;
        }
        i = 0;
    }
    return false;
}


std::string Player::getNick() const
{
    return nick;
}

void Player::setNick(const std::string &value)
{
    nick = value;
}

std::vector<Card> Player::getCards() const
{
    return cards;
}

void Player::setCards(const std::vector<Card> &value)
{
    cards = value;
}

Card Player::getLastCard()
{
	return lastCard;
}

void Player::setLastCard(Card value)
{
	lastCard = value;
}

bool Player::getWin()
{
	return win;
}

void Player::setWin(bool value)
{
	win = value;
}

bool Player::getCanExchange()
{
	return canExchange;
}

void Player::setCanExchange(bool value)
{
	canExchange = value;
}
