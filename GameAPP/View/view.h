#ifndef VIEW_H
#define VIEW_H
#include "Model/card.h"
#include "Model/game.h"
#include "cardview.h"
#include "button.h"
#include "util.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtWidgets>
#include <QObject>
#include <vector>


/*
 * author Adrian Sobolewski
 *
 * View - represents a custom QGraphicsView displaying game GUI
 */

class View : public QGraphicsView
{
    Q_OBJECT
    QGraphicsScene *scene;
    unsigned int windowSize;
    qreal scaleFactor;
    unsigned int cardWidth;
    unsigned int cardHeight;
    static const unsigned int imageHeight = 726;
    static const unsigned int imageWidth = 500;
    enum Position {BOTTOM, LEFT, TOP, RIGHT};

    Game *game;
    std::vector<CardView*> cards;
    std::vector<CardView*> floatingCards;
    std::vector<CardView*> playerCards;
    enum User {OBSERVER, PLAYER};
    User user;

    /**
     * @brief drawCard
     *      draws a card in given coords
     * @param x
     *      x coord
     * @param y
     *      y coord
     * @param p
     *      position - if the card is on the left or right side of the view it will be rotated by 90 or 270 degrees
     * @param floatingCard
     *      describes if the card is floatingCard - if no it won't be visable
     */
    void drawCard(unsigned x, unsigned y, Position p, bool floatingCard);

    /**
     * @brief drawLabel
     *      draws a label in given coords with a given text
     * @param x
     *      x coord
     * @param y
     *      y coord
     * @param text
     */
    void drawLabel(unsigned x, unsigned y, std::string text);

    /**
     * @brief drawCardsLine
     *      draws a cards line (max 3) with player nicks labels on a given position
     * @param ammount
     *      ammount of players in line
     * @param nicks
     *      players nicks
     * @param p
     *      position
     */
    void drawCardsLine(unsigned ammount, std::vector<short> &Ids, Position p);

    /**
     * @brief drawCards
     *      draws cards for all players
     * @param nicks
     *      players nicks
     */
    void drawCards(std::vector<short> nicks);

    /**
     * @brief drawButtons
     *      draws two buttons for controlling the game, one for passing card and one for exchanging card
     */
    void drawButtons();

    /**
     * @brief setPlayerCards
     *      sets playerCards which are the cards of the local player
     */
    void setPlayerCards();

    /**
     * @brief activatePlayerCards
     *      activates player cards so the player can interact with them on his turn
     */
    void activatePlayerCards();

    /**
     * @brief deactivatePlayerCards
     *      deactivates player cards so the player can't interact with them
     */
    void deactivatePlayerCards();

    /**
     * @brief initialize
     *      initializes the View depending on screen size
     */
    void initialize();

    /**
     * @brief update
     *      updates the view depending on given cards, player and floatingCard
     * @param c
     *      players cards from model
     * @param player
     *      current player index
     * @param floatingCard
     *      floating card from model
     */
    void update(std::vector<Card> c, int player, Card floatingCard);

    /**
     * @brief endGame
     *      clears the view and pops a dialog about game end
     */
    void endGame();

public:
    /**
     * @brief View
     *      invokes initialize()
     */
    View();

    /**
     * @brief View
     *      invokes initialize() and sets user and game
     * @param g
     *      game model
     * @param u
     *      user type: 0 = observer, 1 = player
     */
    View(Game *g, unsigned u);
    ~View();

    /**
     * @brief start
     *      starts the game, darws the cards, buttons and updates the view depending on game model
     */
    void start();
    void setUser(unsigned u);

    /**
     * @brief update
     *      invokes void update(std::vector<Card> c, int player, Card floatingCard)
     */
    void update();

public slots:

    void passCard();
    void exchangeCard();
    void cardClicked();
};
#endif // VIEW_H
