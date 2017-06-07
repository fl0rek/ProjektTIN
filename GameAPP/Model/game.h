#ifndef GAME_H
#define GAME_H
#include "card.h"
#include "player.h"
#include "Tlv.h"
#include "tags.h"

/*
 * author Adrian Sobolewski
 *
 * Game class constains game objects such as Deck, GameState, player and functions to check, manipulate and send Messages
 */

class Game
{
    enum Mode {CLIENT, SERVER};
public:
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
        int currentPlayerId = 0;
        Deck deck;
        Card floatingCard;
        bool isStarted = false;
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
         *      gets players cards vector starting from provided player (the temporary players vector is shifted accordingly)
         * @param player
         *      player to be started
         * @return
         */
        std::vector<Card> getPlayersCards();

        /**
         * @brief getCurrentPlayerIndex
         *      gets current player index in players vector
         * @return
         *      index
         */
        unsigned getCurrentPlayerIndex();
        int getCurrentPlayerId();
        unsigned getNextPlayerIndex();

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
    GameState gameState;
private:
    const static int kMaxPlayers = 12;
    int playerId = 0;
    Mode mode;

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

public:

    Game();

    /**
     * @brief Game
     *      creates an empty Game in a provided mode 0 - server 1 - client
     */
    Game(int m);

    /**
     * @brief addPlayer
     *      adds a player to gameState
     * @param id
     *      player id
     */
    void addPlayer(int id);

    /**
     * @brief start
     *      starts the game: deals cards to players, sets isStarted, checks if players can win
     */
    void start();

    /**
     * @brief passCard
     *      simulates a passCard move on gameState and sends it in a Tlv
     * @param c
     *      card
     */
    void passCard(Card c);

    /**
     * @brief exchangeCard
     *      simulates a exchangeCard move on gameState and sends it in a Tlv
     */
    void exchangeCard();

    /**
     * @brief acceptMessage
     *      receives a serialized Tlv and makes suitablbe action depending on message tag
     */
    void acceptMessage(Tlv &buffer);

    /**
     * @brief sendMessage
     *      serializes given GameState and sends it on stdout with a given tag in a Tlv buffer
     * @param gs
     */
    void sendMessage(GameState msg, const unsigned char *t);

    GameState deserializeGameState(std::vector<unsigned char> data);
    Deck deserializeDeck(std::vector<unsigned char> data);
    std::vector<Player*> deserializePlayers(std::vector<unsigned char> data);
    Player *deserializePlayer(std::vector<unsigned char> data);

    std::vector<Card> getPlayersCards();
    unsigned getCurrentPlayerIndex();
    Player *getCurrentPlayer();
    int getCurrentPlayerId();
    int getPlayerId();
    void setPlayerId(int id);
    std::vector<Player*> getPlayers() const;
    bool getIsFinished() const;
    bool getIsStarted() const;
    Card getFloatingCard() const;
    Deck getDeck() const;
    std::vector<int> getPlayersIds();
};

#endif // GAME_H
