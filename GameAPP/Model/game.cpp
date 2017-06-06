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

short Game::GameState::getCurrentPlayerIndex()
{
    return currentPlayerId;
}

Player *Game::GameState::getCurrentPlayer()
{
    return players[currentPlayerId];
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
    addPlayer(1);
    addPlayer(2);
    addPlayer(3);
    addPlayer(4);
    addPlayer(5);
    addPlayer(6);
    addPlayer(7);
    addPlayer(8);
    addPlayer(9);
    addPlayer(10);
    addPlayer(11);
    addPlayer(12);
    start();
//    gameState.addWinner(gameState.players.front());
//    gameState.addWinner(gameState.players.front());

    Message msg;
    msg.gs = gameState;
    sendMessage(msg, tag::game_tags::step);
}

Game::Game(char id)
{
    player = new Player(id);
}

Game::Game(GameState gameStatus)
{
    initialize(gameStatus);
}

void Game::initialize()
{
    gameState.deck = Deck();
    gameState.deck.initialize();
}

void Game::initialize(GameState gs)
{
    gameState = gs;
}

//TODO - dummy simple way updating with whole game state
void Game::update(Game::GameState gs)
{
    gameState = gs;
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
    //msg.t = GS_REQUEST;
    //sendMessage(msg);
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
    //msg.t = CLIENT_START;
    //sendMessage(msg, tag::game_tags::start_game);
}

void Game::passCard(Card c)
{
    Message msg;
    msg.gs = gameState;
    msg.gs.passCard(c);
    if(msg.gs.isFinished)
        ;//msg.t = END_GAME;
    else
        ;//msg.t = PASS_CARD;
    //sendMessage(msg, tag::game_tags::step);
}

void Game::exchangeCard()
{
    Message msg;
    msg.gs = gameState;
    msg.gs.exchangeCard();;
    //msg.t = EXCHANGE;
    //sendMessage(msg, tag::game_tags::step);
}

void Game::terminate()
{
    Message msg;
    msg.gs = gameState;
    //msg.t = TERMINATE;
    //sendMessage(msg, tag::game_tags);
}

//TODO - just a early version
void Game::acceptMessage(Tlv buffer)
{
//    Message msg;
//    if(buffer.isTagPresent(tag::game_tags::add_client))
//    {
//        msg = deserialize(buffer.getAllData());
//        if(msg.t == ADD_PLAYER)
//            addPlayer(msg.id);
//    }

//    if(buffer.isTagPresent(tag::game_tags::start_game))
//    {
//        start();
//    }

//    if(buffer.isTagPresent(tag::game_tags::resync_request))
//    {

//    }

//    if(buffer.isTagPresent(tag::game_tags::step))
//    {
//        msg = deserialize(buffer.getAllData());
//        if(msg.t == PASS_CARD)
//        {
//            if(isValid(msg.gs))
//            {
//                msg.t = UPDATE;
//                sendMessage(msg, tag::game_tags::step_response);
//            }
//        }
//        if(msg.t == EXCHANGE)
//        {
//            if(isValid(msg.gs))
//            {
//                msg.t = UPDATE;
//                sendMessage(msg, tag::game_tags::step_response);
//            }
//        }
//    }

//    if(buffer.isTagPresent(tag::game_tags::step_response))
//    {
//        msg = deserialize(buffer.getAllData());
//        update(msg.gs);

//    }

//    std::cout<<"Message accepted";
//    //Message msg = deserialize(s);
//    Message msg;
//    if(msg.t == ADD_PLAYER)
//        addPlayer(msg.id);
//    else if(msg.t == SERVER_START)
//        start();
//    else
//        switch(msg.t)
//        {
//        case PASS_CARD:
//            if(isValid(msg.gs))
//            {
//                msg.t = UPDATE;
//                sendMessage(msg);
//            }
//            break;
//        case EXCHANGE:
//            if(isValid(msg.gs))
//            {
//                msg.t = UPDATE;
//                sendMessage(msg);
//            }
//        case END_GAME:
//            if(isValid(msg.gs))
//            {
//                update(msg.gs);
//                terminate();
//            }
//            break;
//        case GS_REQUEST:
//            getWholeGameStatus();
//            break;
//        case INITIALIZE:
//            initialize();
//            break;
//        case INIT_FROM_GS:
//            initialize(msg.gs);
//            break;
//        case UPDATE:
//            if(isValid(msg.gs))
//                update(msg.gs);
//            else
//                requestGameState();
//            break;
//        case CLIENT_START:
//            initialize(msg.gs);
//            setPlayer();
//            break;
//        case TERMINATE:
//            gameState = msg.gs;
//            break;
//        default:
//            std::cout<<"Invalid message type";
//            break;
//        }
}

//TODO - it's just sending a message to be accepted by another game instance
void Game::sendMessage(Message msg, unsigned char *t)
{
    std::vector<unsigned char> data = msg.serialize();
    Tlv buffer;
    buffer.add(t, 0, data.size(), data);
    std::vector<unsigned char> full_buffer = buffer.getAllData();
    std::cout<<full_buffer;
}


std::string Game::serialize(Message msg)
{
    sendMessage(msg, tag::game_tags::step);
}

std::string Game::serialize(std::vector<Player*> p)
{

}

std::string Game::serialize(Player p)
{

}

std::vector<unsigned char> Game::Message::serialize()
{
    std::vector<unsigned char> data;
    //data.insert(data.begin(), t);
    std::vector<unsigned char> d = gs.serialize();
    for(std::vector<unsigned char>::iterator it = --d.end(); it >= d.begin(); it--)
        data.insert(data.begin(), (*it));
    data.insert(data.begin(), id);
    return data;
}

Game::Message Game::deserialize(std::vector<unsigned char> data)
{
    Message msg;
    //msg.t = static_cast<MessageType>(data.back());
    //data.pop_back();
    msg.gs = deserializeGameState(data);
    //msg.t = static_cast<MessageType>(data.back());
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


void Game::getWholeGameStatus()
{
    Message msg;
    msg.gs = gameState;
    //msg.t = INIT_FROM_GS;
    //sendMessage(msg);
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


Player *Game::getPlayer()
{
    return player;
}

void Game::setPlayer()
{
    for(Player *p : gameState.players)
        if(p->getId() == player->getId())
        {
            delete player;
            player = p;
        }
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
