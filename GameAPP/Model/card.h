#ifndef CARD_H
#define CARD_H
#include <utility>
#include <boost/serialization/utility.hpp>

/*
 * author Adrian Sobolewski
 *
 * Card represents a pair of enums Rank and Suit
 */

enum Rank : char {
    ACE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK,
    QUEEN,
    KING,
    NO_RANK
};

enum Suit : char {
	HEARTS,
	DIAMONDS,
	CLUBS,
	SPADES,
    NO_SUIT
};

typedef std::pair<Rank, Suit> Card;

bool compare(Card c1, Card c2);

#endif // CARD_H
