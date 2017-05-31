#ifndef PLAYER_H
#define PLAYER_H
#include "card.h"

#include <vector>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

/*
 * author Adrian Sobolewski
 *
 * Player class contains player cards vector, his nick/id and bools defining wheter he won the game and can exchange his last card
 */

class Player
{
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int )
    {
        ar & nick;
        ar & cards;
        ar & lastCard;
        ar & win;
        ar & canExchange;
    }

    std::string nick;
    std::vector<Card> cards;
    Card lastCard;
	bool win = false;
	bool canExchange = false;
public:
    /**
     * @brief Player
     *      creates a empty player
     */
    Player();

    /**
     * @brief Player
     *      creates a player with given nick/id
     * @param nick
     */
    Player(std::string nick);

    /**
     * @brief Player
     *      creates a player with given nick/id and given cards
     * @param nick
     * @param cards
     */
    Player(std::string nick, std::vector<Card> cards);

    /**
     * @brief giveCard
     *      adds a given card to cards vector
     * @param c
     *      card
     */
    void giveCard(Card c);

    /**
     * @brief removeCard
     *      removes a given card from cards vector
     * @param c
     *      card
     */
    void removeCard(Card c);

    /**
     * @brief hasCard
     *      checks if player has a given card
     * @param card
     * @return
     *      true or false
     */
    bool hasCard(Card card);

    /**
     * @brief hasThree
     *      checks if player has three cards with the same rank or three cards with the same suit
     * @return
     *      true or false
     */
    bool hasThree();


    std::string getNick() const;
	void setNick(const std::string &value);
    std::vector<Card> getCards() const;
	void setCards(const std::vector<Card> &value);
	Card getLastCard();
	void setLastCard(Card value);
	bool getWin();
	void setWin(bool value);
	bool getCanExchange();
	void setCanExchange(bool value);
};

#endif // PLAYER_H
