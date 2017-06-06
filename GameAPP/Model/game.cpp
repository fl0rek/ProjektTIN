#include "game.h"

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <sstream>

/*
 * Deck
 */

Game::Deck::Deck()
{

}

Card Game::Deck::getCard()
{
    if(cards.size() == 0 && rejectedCards.size() != 0)
        reshuffle();
    Card card = cards.front();
    cards.erase(cards.begin());
    return card;
}

void Game::Deck::putCard(Card card)
{
    rejectedCards.push_back(card);
}

Card Game::Deck::exchangeCard(Card card)
{
    putCard(card);
    return getCard();
}

void Game::Deck::shuffle(std::vector<Card> &v)
{
    srand(time(NULL));
    std::random_shuffle(v.begin(), v.end());
}

void Game::Deck::reshuffle()
{
    srand(time(NULL));
    std::random_shuffle(rejectedCards.begin(), rejectedCards.end());
    for(Card c : rejectedCards)
        cards.push_back(c);
    rejectedCards.clear();
}

void Game::Deck::initialize()
{
    cards.clear();
    for(int i = 0; i < 13; i++)
        for(int j = 0; j < 4; j++)
        {
            Card card = std::make_pair(static_cast<Rank>(i), static_cast<Suit>(j));
            cards.push_back(card);
        }
    shuffle(cards);
}

std::vector<unsigned char> Game::Deck::serialize()
{
    std::vector<unsigned char> data;
    data.push_back(cards.size());
    for(Card c : cards)
    {
        data.insert(data.begin(), c.first);
        data.insert(data.begin(), c.second);
    }

    data.insert(data.begin(), rejectedCards.size());
    for(Card c : rejectedCards)
    {
        data.insert(data.begin(), c.first);
        data.insert(data.begin(), c.second);
    }
    data.push_back(data.size());
    return data;
}

std::vector<Card> Game::Deck::getCards()
{
    return cards;
}

void Game::Deck::setCards(const std::vector<Card> &value)
{
    cards = value;
}

bool Game::Deck::operator ==(const Game::Deck &d) const
{
    if(d.getRejectedCards().size() != this->rejectedCards.size())
        return false;
    if(d.getCardsVector().size() != this->cards.size())
        return false;
    for(unsigned i = 0; i != this->rejectedCards.size() || i != d.getRejectedCards().size(); i++)
        if(this->rejectedCards[i].first != d.getRejectedCards()[i].first || this->rejectedCards[i].second != d.getRejectedCards()[i].second)
            return false;
    for(unsigned i = 0; i != this->cards.size() || i != d.getCardsVector().size(); i++)
        if(this->cards[i].first != d.getCardsVector()[i].first || this->cards[i].second != d.getCardsVector()[i].second)
            return false;
    return true;
}

std::vector<Card> Game::Deck::getCardsVector() const
{
    return cards;
}

std::vector<Card> Game::Deck::getRejectedCards() const
{
    return rejectedCards;
}

void Game::Deck::setRejectedCards(const std::vector<Card> &value)
{
    rejectedCards = value;
}


/*
 * GameState
 */

void Game::GameState::passCard(Card c)
{
    if(getCurrentPlayer()->hasCard(c))
    {
        // if it is players' first move his last card should be the card he is passing
        if(getCurrentPlayer()->getLastCard().first == NO_RANK && getCurrentPlayer()->getLastCard().second == NO_SUIT)
            getCurrentPlayer()->setLastCard(c);
        getCurrentPlayer()->removeCard(c);
        nextTurn();
        getCurrentPlayer()->giveCard(c);
        if (getCurrentPlayer()->getLastCard() == c)
            getCurrentPlayer()->setCanExchange(true);
        else
            getCurrentPlayer()->setCanExchange(false);
        if (getCurrentPlayer()->hasThree())
            getCurrentPlayer()->setWin(true);
        else
            getCurrentPlayer()->setWin(false);
        getCurrentPlayer()->setLastCard(c);
        floatingCard = c;
    }
}

void Game::GameState::exchangeCard()
{
    Card c = deck.exchangeCard(floatingCard);
    getCurrentPlayer()->giveCard(c);
    getCurrentPlayer()->removeCard(floatingCard);
    getCurrentPlayer()->setCanExchange(false);
    getCurrentPlayer()->setLastCard(std::make_pair(NO_RANK, NO_SUIT));
    getCurrentPlayer()->setWin(getCurrentPlayer()->hasThree());
    floatingCard = c;
}

void Game::GameState::nextTurn()
{
    for(unsigned i = 0; i < players.size(); i++)
        if((players[i]->getId() == currentPlayerId) && (i + 1 < players.size()))
        {
            if(getCurrentPlayer()->getWin())
                addWinner(getCurrentPlayer());
            currentPlayerId++;
            break;
        }
        else if (i + 1 >= players.size())
        {
            if(getCurrentPlayer()->getWin())
                addWinner(getCurrentPlayer());
            currentPlayerId = 0;
            break;
        }
}

void Game::GameState::addWinner(Player *p)
{
    winners.push_back(p);
    if(players.size() <= 1)
        isFinished = true;
    players.erase(std::remove(players.begin(), players.end(), p), players.end());
}

std::vector<Card> Game::GameState::getPlayersCards()
{
    std::vector<Card> cards;
    for(Player *p : players)
        for(Card c : p->getCards())
            if(!(c.first == floatingCard.first && c.second == floatingCard.second))
                    cards.push_back(c);
    return cards;
}

//TO UPDATE
std::vector<Card> Game::GameState::getPlayersCards(Player *player)
{
    std::vector<Player*> vec = players;
    std::vector<Card> cards;
    for(std::vector<Player*>::iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if((*it) == player)
        {
            std::rotate(vec.begin(), it, vec.end());
            break;
        }
    }
    for(Player *p : vec)
        for(Card c : p->getCards())
            if(!(c.first == floatingCard.first && c.second == floatingCard.second))
                    cards.push_back(c);
    return cards;

}

unsigned Game::GameState::getCurrentPlayerIndex()
{
    for(unsigned i = 0; i < players.size(); i++)
        if(players[i]->getId() == currentPlayerId)
            return i;
}

unsigned Game::GameState::getCurrentPlayerId()
{
    return currentPlayerId;
}

unsigned Game::GameState::getNextPlayerIndex()
{
    if(players.back()->getId() == currentPlayerId)
        return 0;
    for(unsigned i = 0; i < players.size(); i++)
        if(players[i]->getId() == currentPlayerId)
            return ++i;
}

Player *Game::GameState::getCurrentPlayer()
{
    return players[getCurrentPlayerIndex()];
}

void Game::GameState::dealCards()
{
    for(unsigned i = 0; i < players.size() * 3; i++)
        dealCard(players[i % players.size()]);
    dealCard(players.front());
    floatingCard = players.front()->getCards().back();
}

void Game::GameState::dealCard(Player *p)
{
    p->giveCard(deck.getCard());
}

std::vector<unsigned char> Game::GameState::serialize()
{
    std::vector<unsigned char> data;
    data.insert(data.begin(), isStarted);
    data.insert(data.begin(), isFinished);
    data.insert(data.begin(), currentPlayerId);
    data.insert(data.begin(), floatingCard.first);
    data.insert(data.begin(), floatingCard.second);

    std::vector<unsigned char> d = deck.serialize();
    for(std::vector<unsigned char>::iterator it = --d.end(); it >= d.begin(); it--)
        data.insert(data.begin(), (*it));

    d = serializePlayers(players);
    for(std::vector<unsigned char>::iterator it = --d.end(); it >= d.begin(); it--)
        data.insert(data.begin(), (*it));

    d = serializePlayers(winners);
    for(std::vector<unsigned char>::iterator it = --d.end(); it >= d.begin(); it--)
        data.insert(data.begin(), (*it));

    return data;

}

std::vector<unsigned char> Game::GameState::serializePlayers(std::vector<Player*> players)
{
    std::vector<unsigned char> data;
    for(Player *p : players)
    {
        std::vector<unsigned char> pl = p->serialize();
        for(std::vector<unsigned char>::iterator it = --pl.end(); it >= pl.begin(); it--)
            data.insert(data.begin(), (*it));
    }

    data.push_back(players.size());
    data.push_back(data.size());
    return data;
}


/*
 * Game
 */
Game::Game()
{

}

Game::Game(int m)
{
    mode = static_cast<Mode>(m);
}

Game::Game(GameState gameStatus)
{
    //initialize(gameStatus);
}

Player *Game::getPlayer() const
{
    return player;
}

void Game::setPlayer(Player *value)
{
    player = value;
}

void Game::initialize()
{
    gameState.deck = Deck();
    gameState.deck.initialize();
}

// TO check
bool Game::isValid(GameState gs)
{
    return checkPlayers(gs) && checkCurrentPlayer(gs) && checkCards(gs);
}


bool Game::checkPlayers(Game::GameState gs)
{
    if(gs.players.size() != gameState.players.size())
        return false;
    for(unsigned i = 0; i < gs.players.size() && i < gameState.players.size(); i++)
        if(gs.players[i] != gameState.players[i])
            return false;
    return true;
}

bool Game::checkCurrentPlayer(Game::GameState gs)
{
    return gs.currentPlayerId == gameState.currentPlayerId;
}

bool Game::checkCards(Game::GameState gs)
{
    if(gs.getPlayersCards().size() != gameState.getPlayersCards().size())
        return false;
    for(unsigned i = 0; i < gs.getPlayersCards().size() && i < gameState.getPlayersCards().size(); i++)
        if(gs.getPlayersCards()[i] != gameState.getPlayersCards()[i])
            return false;
    return true;
}

bool Game::checkDeck(Game::GameState gs)
{
    return gs.deck == gameState.deck;
}

void Game::requestGameState()
{
    Message msg;
    sendMessage(msg, tag::game_tags::resync_request);
}

void Game::addPlayer(char id)
{
    if(gameState.players.size() < kMaxPlayers)
    {
        gameState.players.push_back(new Player(id));
    }
}

void Game::start()
{
    gameState.deck.initialize();
    gameState.dealCards();
    for(Player *p : gameState.players)
        if(p->hasThree())
            gameState.addWinner(p);
    gameState.isStarted = true;
    gameState.currentPlayerId = 1;
    Message msg;
    msg.gs = gameState;
    sendMessage(msg, tag::game_tags::step);
}

void Game::passCard(Card c)
{
    Message msg;
    msg.gs = gameState;
    msg.gs.passCard(c);
    sendMessage(msg, tag::game_tags::step);
}

void Game::exchangeCard()
{
    Message msg;
    msg.gs = gameState;
    msg.gs.exchangeCard();;
    sendMessage(msg, tag::game_tags::step);
}

void Game::terminate()
{

}

//TODO - just a early version
void Game::acceptMessage(Tlv buffer)
{
    Message msg;
    if(mode == SERVER)
    {
        if(buffer.isTagPresent(tag::game_tags::add_client))
        {
            std::vector<unsigned char> c = buffer.getTagData(tag::internal_tags::client_id);
            int id = (c[0] << 24) | (c[1] << 16) | (c[2] << 8) | c[3];
            addPlayer(id);
        }
        if(buffer.isTagPresent(tag::game_tags::start_game))
        {
            start();
        }
        if(buffer.isTagPresent(tag::game_tags::step))
        {
            msg = deserialize(buffer.getAllData());
            if(isValid(msg.gs))
                if(!msg.gs.isFinished)
                    sendMessage(msg, tag::game_tags::step);
                else
                    sendMessage(msg, tag::game_tags::terminate);
            else
                sendMessage(msg, tag::game_tags::invalid_step);
        }
    }

    if(mode == CLIENT)
    {
        if(buffer.isTagPresent(tag::game_tags::invalid_step));
        if(buffer.isTagPresent(tag::game_tags::step))
        {
            msg = deserialize(buffer.getAllData());
            gameState = msg.gs;
        }
        if(buffer.isTagPresent(tag::game_tags::terminate))
        {

        }
    }

}

//TODO - it's just sending a message to be accepted by another game instance
void Game::sendMessage(Message msg, const unsigned char *t)
{
    std::vector<unsigned char> data = msg.serialize();
    Tlv buffer;
    buffer.add(t, 0, data.size(), data.data());
    std::vector<unsigned char> full_buffer = buffer.getAllData();
    for_each(full_buffer.begin(), full_buffer.end(), [](auto element)
    {
         std::cout<<element;
    });
    std::cout<<std::endl;
}

std::vector<unsigned char> Game::Message::serialize()
{
    std::vector<unsigned char> data;
    std::vector<unsigned char> d = gs.serialize();
    for(std::vector<unsigned char>::iterator it = --d.end(); it >= d.begin(); it--)
        data.insert(data.begin(), (*it));
    data.insert(data.begin(), id);
    return data;
}

Game::Message Game::deserialize(std::vector<unsigned char> data)
{
    Message msg;
    msg.gs = deserializeGameState(data);
    return msg;
}

Game::GameState Game::deserializeGameState(std::vector<unsigned char> data)
{
    GameState gs;

    gs.isStarted = data.back();
    data.pop_back();

    gs.isFinished = data.back();
    data.pop_back();

    gs.currentPlayerId = data.back();
    data.pop_back();

    Card c;
    c.first = static_cast<Rank>(data.back());
    data.pop_back();
    c.second = static_cast<Suit>(data.back());
    data.pop_back();
    gs.floatingCard = c;

    unsigned n = 0;
    n = static_cast<unsigned>(data.back());
    data.pop_back();
    std::vector<unsigned char>::const_iterator first = data.end() - n;
    std::vector<unsigned char>::const_iterator last = data.end();
    std::vector<unsigned char> deckData(first, last);
    gs.deck = deserializeDeck(deckData);

    data.erase(first, last);
    n = static_cast<unsigned>(data.back());
    data.pop_back();
    first = data.end() - n;
    last = data.end();
    std::vector<unsigned char> playersData(first, last);
    gs.players = deserializePlayers(playersData);

    data.erase(first, last);
    n = static_cast<unsigned>(data.back());
    data.pop_back();
    first = data.end() - n;
    last = data.end();
    std::vector<unsigned char> winnersData(first, last);
    gs.winners = deserializePlayers(winnersData);

    return gs;
}

Game::Deck Game::deserializeDeck(std::vector<unsigned char> data)
{
    Deck deck;
    std::vector<Card> cards;
    Card c;

    unsigned cardsSize = data.back();
    data.pop_back();
    for(unsigned i = 0; i < cardsSize; i++)
    {
        c.first = static_cast<Rank>(data.back());
        data.pop_back();
        c.second = static_cast<Suit>(data.back());
        data.pop_back();
        cards.insert(cards.begin(), c);
    }
    std::reverse(cards.begin(), cards.end());
    deck.setCards(cards);
    cards.clear();

    cardsSize = data.back();
    data.pop_back();
    for(unsigned i = 0; i < cardsSize; i++)
    {
        c.first = static_cast<Rank>(data.back());
        data.pop_back();
        c.second = static_cast<Suit>(data.back());
        data.pop_back();
        cards.insert(cards.begin(), c);
    }
    std::reverse(cards.begin(), cards.end());
    deck.setRejectedCards(cards);
    return deck;
}

std::vector<Player *> Game::deserializePlayers(std::vector<unsigned char> data)
{
    std::vector<Player*> players;
    unsigned playersSize = data.back();
    data.pop_back();

    for(unsigned i = 0; i < playersSize; i++)
    {
        unsigned n = 0;
        n = static_cast<unsigned>(data.back());
        data.pop_back();
        std::vector<unsigned char>::const_iterator first = data.end() - n;
        std::vector<unsigned char>::const_iterator last = data.end();
        std::vector<unsigned char> playerData(first, last);
        players.push_back(deserializePlayer(playerData));
        data.erase(first, last);
    }

    return players;
}

Player *Game::deserializePlayer(std::vector<unsigned char> data)
{
    Player *p = new Player();

    p->setId(data.back());
    data.pop_back();

    p->setWin(data.back());
    data.pop_back();

    p->setCanExchange(data.back());
    data.pop_back();

    Card c;
    c.first = static_cast<Rank>(data.back());
    data.pop_back();
    c.second = static_cast<Suit>(data.back());
    data.pop_back();
    p->setLastCard(c);

    unsigned n = data.back();
    data.pop_back();

    for(unsigned i = 0; i < n; i++)
    {
        c.first = static_cast<Rank>(data.back());
        data.pop_back();
        c.second = static_cast<Suit>(data.back());
        data.pop_back();
        p->giveCard(c);
    }
    return p;
}

std::vector<Card> Game::getPlayersCards()
{
    return gameState.getPlayersCards(player);
}


short Game::getCurrentPlayerIndex()
{
    return gameState.currentPlayerId;
}


Player *Game::getCurrentPlayer()
{
    return gameState.getCurrentPlayer();
}


unsigned Game::getPlayerId()
{
    return playerId;
}

void Game::setPlayerId(unsigned id)
{
    playerId = id;
}


std::vector<Player *> Game::getPlayers() const
{
    return gameState.players;
}


bool Game::getIsFinished() const
{
    return gameState.isFinished;
}


Card Game::getFloatingCard() const
{
    return gameState.floatingCard;
}


Game::Deck Game::getDeck() const
{
    return gameState.deck;
}
