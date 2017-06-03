#include "tst_playertests.h"
#include "card.h"
#include "player.h"

#include <QObject>
#include <vector>
#include <utility>

PlayerTests::PlayerTests()
{

}

void PlayerTests::playerConstructor1()
{
    Player p("a");
    Card c = std::make_pair(NO_RANK, NO_SUIT);

    //QCOMPARE(p.getNick(), "a");
    QCOMPARE(compare(p.getLastCard(), c), true);
}

void PlayerTests::playerConstructor2()
{
    std::vector<Card> cards;
    cards.push_back(std::make_pair(TWO, HEARTS));
    cards.push_back(std::make_pair(ACE, SPADES));
    cards.push_back(std::make_pair(TEN, CLUBS));
    Player p("a", cards);

    QCOMPARE(cards.size(), p.getCards().size());
    for(unsigned i = 0; i < cards.size(), i < p.getCards().size(); i++)
        QCOMPARE(compare(cards[i], p.getCards()[i]), true);

}

void PlayerTests::playerGiveCard()
{
    Player p("a");
    Card c = std::make_pair(TWO, HEARTS);
    p.giveCard(c);

    QCOMPARE(compare(p.getCards().back(), c), true);
}

void PlayerTests::playerRemoveCard()
{
    Player p("a");
    Card c = std::make_pair(TWO, HEARTS);
    p.giveCard(c);
    p.giveCard(std::make_pair(TWO, CLUBS));
    p.giveCard(std::make_pair(THREE, DIAMONDS));
    p.giveCard(std::make_pair(FOUR, SPADES));
    p.removeCard(c);

    for(Card card : p.getCards())
        QCOMPARE(compare(card, c), false);
}

void PlayerTests::playerHasCard()
{
    Player p("a");
    Card c = std::make_pair(TWO, HEARTS);
    p.giveCard(std::make_pair(THREE, DIAMONDS));
    p.giveCard(std::make_pair(FOUR, SPADES));
    p.giveCard(c);
    Card fc = std::make_pair(ACE, HEARTS);

    QCOMPARE(p.hasCard(c), true);
    QCOMPARE(p.hasCard(fc), false);
}

void PlayerTests::playerHasThreeTrue()
{
    Player p("a");
    p.giveCard(std::make_pair(TWO, HEARTS));
    p.giveCard(std::make_pair(THREE, HEARTS));
    p.giveCard(std::make_pair(FOUR, HEARTS));

    QCOMPARE(p.hasThree(), true);
}

void PlayerTests::playerHasThreeFalse()
{
    Player p("a");
    p.giveCard(std::make_pair(TWO, HEARTS));
    p.giveCard(std::make_pair(THREE, HEARTS));
    p.giveCard(std::make_pair(FOUR, CLUBS));

    QCOMPARE(p.hasThree(), false);
}

