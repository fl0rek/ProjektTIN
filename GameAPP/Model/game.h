#ifndef GAME_H
#define GAME_H
#include "card.h"
#include "player.h"
#include "util.h"
#include "Tlv.h"
#include "tags.h"

/*
 * author Adrian Sobolewski
 *
 * Game class constains game objects such as Deck, GameState, player and functions to check, manipulate and send Messages
 */

class Game
{
public:
    enum MessageType {NO_TYPE, PASS_CARD, EXCHANGE, END_GAME, GS_REQUEST, ADD_PLAYER, SERVER_START, INITIALIZE, INIT_FROM_GS, CLIENT_START, UPDATE, TERMINATE};
    class Deck
    {
    public:

        /**
         * @brief Deck
         *      creates a empty deck
         */
        Deck();

        /**
         * @brief getCard
         *      gets first card from the cards vector, if the vector is empty it reshuffle rejectedCards and then gets first card
         * @return
         *      first card from deck
         */
        Card getCard();

        /**
         * @brief putCard
         *      adds a card to rejectedCards vector
         * @param card
         *      card that will be put
         */
        void putCard(Card card);

        /**
         * @brief exchangeCard
         *      exchanges given card
         * @param card
         *      card that will be exchanged
         * @return
         *      first card from the cards vector
         */
        Card exchangeCard(Card card);

        /**
         * @brief shuffle
         *      shuffles a vector of cards
         * @param v
         *      vector of cards
         */
        void shuffle(std::vector<Card> &v);

        /**
         * @brief reshuffle
         *      shuffles the rejectedCards vector, adds it to cards vector and clears it
         */
        void reshuffle();

        /**
         * @brief initialize
         *      creates pairs of every Rank and Suit enums, adds them to cards vector and then shuffles the vector
         */
        void initialize();
        std::vector<unsigned char> serialize();
        std::vector<Card> getCards();
        void setCards(const std::vector<Card> &value);
        bool operator == (const Deck &d) const;
        std::vector<Card> getCardsVector() const;
        std::vector<Card> getRejectedCards() const;
        void setRejectedCards(const std::vector<Card> &value);

    private:
        std::vector<Card> cards;
        std::vector<Card> rejectedCards;
    };

    struct GameState
    {
        std::vector<Player*> players;
        std::vector<Player*> winners;
        short currentPlayerId;
        Deck deck;
        Card floatingCard;
        bool isStarted;
        bool isFinished = false;

        /**
         * @brief passCard
         *      simulates a card pass from player to player updating current and next player cards,
         *      states and the floatingCard
         * @param c
         *      card to be passed
         */
        void passCard(Card c);

        /**
         * @brief exchangeCard
         *      card exchange for current player
         */
        void exchangeCard();

        /**
         * @brief nextTurn
         *      changes currentPlayer to next one
         */
        void nextTurn();

        /**
         * @brief addWinner
         *      adds a player to winner vector and erases him from players vector,
         *      updating isFinished
         * @param p
         *      player
         */
        void addWinner(Player *p);

        /**
         * @brief getPlayersCards
         *      gets players cards vector in order directed by players vector
         * @return
         *      players cards vector
         */
        std::vector<Card> getPlayersCards();

        /**
         * @brief getPlayersCards
         *      gets players cards vector starting from provided player (the temporary players vector is shifted accordingly)
         * @param player
         *      player to be started
         * @return
         */
        std::vector<Card> getPlayersCards(Player *player);

        /**
         * @brief getCurrentPlayerIndex
         *      gets current player index in players vector
         * @return
         *      index
         */
        short getCurrentPlayerIndex();

        Player *getCurrentPlayer();

        /**
         * @brief dealCards
         *      deals 3 cards to every player and then one card to starting player
         */
        void dealCards();

        /**
         * @brief dealCard
         *      deals card to a player from deck cards
         * @param p
         *      player
         */
        void dealCard(Player *p);

        std::vector<unsigned char> serialize();
        std::vector<unsigned char> serializePlayers(std::vector<Player *> players);
    };

    struct Message
    {
        MessageType t;
        GameState gs;
        short id;
        std::vector<unsigned char> serialize();
     };

private:
    const static int kMaxPlayers = 12;
    GameState gameState;
    Player *player;

    /**
     * @brief initialize
     *      creates new deck and initializes it
     */
    void initialize();

    /**
     * @brief initialize
     *      initializes from given GameState
     * @param gs
     *      GameState
     */
    void initialize(GameState gs);

    void update(GameState gs);

    /**
     * @brief isValid
     *      checks if given GameState is valid comparing to actual GameState
     * @param gs
     *      GameState
     * @return
     *      true or false
     */
    bool isValid(GameState gs);

    /**
     * @brief checkPlayers
     *      checks if the players in provided Game State are the same in local gameState
     * @param gs
     *      game state to check
     * @return
     */
    bool checkPlayers(GameState gs);

    /**
     * @brief checkCurrentPlayer
     *      checks if the currentPlayer in provided Game State is the same in local gameState
     * @param gs
     *      Game State to check
     * @return
     */
    bool checkCurrentPlayer(GameState gs);

    /**
     * @brief checkCards
     *      checks if the cards in provided Game State are the same in local gameState
     * @param gs
     *      Game State to check
     * @return
     */
    bool checkCards(GameState gs);

    /**
     * @brief checkDeck
     *      checks if the deck in provided Game State is the same in local gameState
     * @param gs
     *      Game State to check
     * @return
     */
    bool checkDeck(GameState gs);

    /**
     * @brief requestGameState
     *      sends a message requesting current GameState
     */
    void requestGameState();


public:
    /**
     * @brief Game
     *      creates an empty Game
     */
    Game();

    /**
     * @brief Game
     *      invokes initialize(GameState gs)
     * @param gameState
     *
     */
    Game(GameState gameState);

    /**
     * @brief Game
     *      invokes initialize and creates a player for local player
     * @param id
     *      local player id
     */
    Game(char id);

    /**
     * @brief addPlayer
     *      adds a player to gameState
     * @param id
     *      player id
     */
    void addPlayer(short id);

    /**
     * @brief start
     *      starts the game: deals cards to players, sets isStarted, checks if players can win
     */
    void start();

    /**
     * @brief passCard
     *      simulates a passCard move on gameState and sends it in a Message
     * @param c
     *      card
     */
    void passCard(Card c);

    /**
     * @brief exchangeCard
     *      simulates a exchangeCard move on gameState and sends it in a Message
     */
    void exchangeCard();


    /**
     * @brief terminate
     *      sends a message to clients about game end
     */
    void terminate();

    /**
     * @brief acceptMessage
     *      receives a serialized Message and makes suitablbe action depending on message type
     * @param s
     *      serialized Message
     */
    void acceptMessage(Tlv buffer);

    /**
     * @brief sendMessage
     *      serializes given Message and sends it on stdout
     * @param gs
     */
    void sendMessage(Message msg);

    /**
     * @brief serialize
     *      serializes a given Message to a string representation
     * @param gs
     * @return
     *      returns a string representing serialized Message
     */
    std::string serialize(Message msg);
    std::string serialize(std::vector<Player*> p);
    std::string serialize(Player p);

    /**
     * @brief deserialize
     *      deserializes a given string to a Message
     * @param s
     * @return
     *      returns a Message deserialized from a string
     */
    Message deserialize(std::vector<unsigned char> data);
    GameState deserializeGameState(std::vector<unsigned char> data);
    Deck deserializeDeck(std::vector<unsigned char> data);
    std::vector<Player*> deserializePlayers(std::vector<unsigned char> data);
    Player *deserializePlayer(std::vector<unsigned char> data);


    void getWholeGameStatus();
    void sendWholeGameStatus(std::string s);
    std::vector<Card> getPlayersCards();
    short getCurrentPlayerIndex();
    Player *getCurrentPlayer();
    Player *getPlayer();
    void setPlayer();
    std::vector<Player*> getPlayers() const;
    bool getIsFinished() const;
    Card getFloatingCard() const;
    Deck getDeck() const;
};

#endif // GAME_H
