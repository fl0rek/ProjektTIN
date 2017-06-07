#include "player.h"

#include <string>
#include <algorithm>


Player::Player()
{
    
}


Player::Player(int id)
{
    this->id = id;
    lastCard = std::make_pair(NO_RANK, NO_SUIT);
}


Player::Player(int id, std::vector<Card> cards) : id(id), cards{ std::move(cards) }{}


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

std::vector<unsigned char> Player::serialize()
{
    std::vector<unsigned char> data;
    data.insert(data.begin(), id>>24);
    data.insert(data.begin(), id>>16);
    data.insert(data.begin(), id>>8);
    data.insert(data.begin(), id);
    data.insert(data.begin(), win);
    data.insert(data.begin(), canExchange);
    data.insert(data.begin(), lastCard.first);
    data.insert(data.begin(), lastCard.second);
    for(Card c : cards)
    {
        data.insert(data.begin(), c.first);
        data.insert(data.begin(), c.second);
    }
    data.insert(data.begin() + cards.size() * 2, cards.size());
    data.push_back(data.size());
    return data;
}


int Player::getId() const
{
    return id;
}

void Player::setId(int value)
{
    this->id = value;
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
