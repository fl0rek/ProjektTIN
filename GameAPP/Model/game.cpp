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


/*
 * GameState
 */

void Game::GameState::passCard(Card c)
{
    if(currentPlayer->hasCard(c))
    {
        // if it is players' first move his last card should be the card he is passing
        if(currentPlayer->getLastCard().first == NO_RANK && currentPlayer->getLastCard().second == NO_SUIT)
            currentPlayer->setLastCard(c);
        currentPlayer->removeCard(c);
        nextTurn();
        currentPlayer->giveCard(c);
        if (currentPlayer->getLastCard() == c)
            currentPlayer->setCanExchange(true);
        else
            currentPlayer->setCanExchange(false);
        if (currentPlayer->hasThree())
            currentPlayer->setWin(true);
        else
            currentPlayer->setWin(false);
        currentPlayer->setLastCard(c);
        floatingCard = c;
    }
}


void Game::GameState::exchangeCard()
{
    Card c = deck.exchangeCard(floatingCard);
    currentPlayer->giveCard(c);
    currentPlayer->removeCard(floatingCard);
    currentPlayer->setCanExchange(false);
    currentPlayer->setLastCard(std::make_pair(NO_RANK, NO_SUIT));
    currentPlayer->setWin(currentPlayer->hasThree());
    floatingCard = c;
}


void Game::GameState::nextTurn()
{
    for(unsigned i = 0; i < players.size(); i++)
        if((players[i]->getNick() == currentPlayer->getNick()) && (i + 1 < players.size()))
        {
            if(currentPlayer->getWin())
                addWinner(currentPlayer);
            currentPlayer = players[i+1];
            break;
        }
        else if (i + 1 >= players.size())
        {
            if(currentPlayer->getWin())
                addWinner(currentPlayer);
            currentPlayer = players.front();
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


int Game::GameState::getCurrentPlayerIndex()
{
    unsigned i = 0;
    for(Player *p : players)
    {
        if(p->getNick() == currentPlayer->getNick())
            break;
        i++;
    }
    if(i >= players.size())
        return -1;
    return i;
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


/*
 * Game
 */
Game::Game()
{

}


Game::Game(std::string id)
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
    return gs.currentPlayer == gameState.currentPlayer;
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
    msg.t = GS_REQUEST;
    sendMessage(msg);
}


void Game::addPlayer(std::string id)
{
    if(gameState.players.size() < kMaxPlayers)
    {
        gameState.players.push_back(new Player(id));
    }
}


void Game::start()
{
    gameState.dealCards();
    for(Player *p : gameState.players)
        p->setWin(p->hasThree());
    gameState.isStarted = true;
    gameState.currentPlayer = gameState.players.front();

    Message msg;
    msg.gs = gameState;
    msg.t = CLIENT_START;
    sendMessage(msg);
}


void Game::passCard(Card c)
{
    Message msg;
    msg.gs = gameState;
    msg.gs.passCard(c);
    if(msg.gs.isFinished)
        msg.t = END_GAME;
    else
        msg.t = PASS_CARD;
    sendMessage(msg);
}


void Game::exchangeCard()
{
    Message msg;
    msg.gs = gameState;
    msg.gs.exchangeCard();;
    msg.t = EXCHANGE;
    sendMessage(msg);
}


void Game::terminate()
{
    Message msg;
    msg.gs = gameState;
    msg.t = TERMINATE;
    sendMessage(msg);
}

//TODO - just a early version
void Game::acceptMessage(std::string s)
{
    std::cout<<"Message accepted";
    Message msg = deserialize(s);
    if(msg.t == ADD_PLAYER)
        addPlayer(msg.s);
    else if(msg.t == SERVER_START)
        start();
    else
        switch(msg.t)
        {
        case PASS_CARD:
            if(isValid(msg.gs))
            {
                msg.t = UPDATE;
                sendMessage(msg);
            }
            break;
        case EXCHANGE:
            if(isValid(msg.gs))
            {
                msg.t = UPDATE;
                sendMessage(msg);
            }
        case END_GAME:
            if(isValid(msg.gs))
            {
                update(msg.gs);
                terminate();
            }
            break;
        case GS_REQUEST:
            getWholeGameStatus();
            break;
        case INITIALIZE:
            initialize();
            break;
        case INIT_FROM_GS:
            initialize(msg.gs);
            break;
        case UPDATE:
            if(isValid(msg.gs))
                update(msg.gs);
            else
                requestGameState();
            break;
        case CLIENT_START:
            initialize(msg.gs);
            setPlayer();
            break;
        case TERMINATE:
            gameState = msg.gs;
            break;
        default:
            std::cout<<"Invalid message type";
            break;
        }
}

//TODO - it's just sending a message to be accepted by another game instance
void Game::sendMessage(Message msg)
{
    std::cout<<serialize(msg);
}


std::string Game::serialize(Message msg)
{
    std::stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << msg;
    return ss.str();
}


Game::Message Game::deserialize(std::string s)
{
    Message msg;
    std::stringstream ss;
    ss << s;
    boost::archive::text_iarchive ia(ss);
    ia >> msg;
    return msg;
}


void Game::getWholeGameStatus()
{
    Message msg;
    msg.gs = gameState;
    msg.t = INIT_FROM_GS;
    sendMessage(msg);
}


std::vector<Card> Game::getPlayersCards()
{
    return gameState.getPlayersCards(player);
}


int Game::getCurrentPlayerIndex()
{
    return gameState.getCurrentPlayerIndex();
}


Player *Game::getCurrentPlayer()
{
    return gameState.currentPlayer;
}


Player *Game::getPlayer()
{
    return player;
}

void Game::setPlayer()
{
    for(Player *p : gameState.players)
        if(p->getNick() == player->getNick())
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
