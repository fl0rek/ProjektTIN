/*
 * author Adrian Sobolewski
 */

#include "view.h"

#include <QScreen>
#include <QGraphicsTextItem>
#include <fstream>
#include <QDebug>

void View::drawCard(unsigned x, unsigned y, View::Position p, bool floatingCard)
{
    CardView *c = new CardView();
    c->setScale(scaleFactor);
    c->setPos(x, y);
    c->setVisible(false);
    if(p == LEFT)
        c->setRotation(90);
    if(p == RIGHT)
        c->setRotation(270);
    if(floatingCard)
    {
        floatingCards.push_back(c);
        c->setVisible(false);
    }
    else
        cards.push_back(c);
    connect(c, SIGNAL(clicked()), this, SLOT(cardClicked()));
    scene->addItem(c);
}

void View::drawLabel(unsigned x, unsigned y)
{
    QGraphicsTextItem *label = new QGraphicsTextItem;
    label->setPos(x, y);
    scene->addItem(label);
    labels.push_back(label);
}

void View::drawCardsLine(unsigned ammount, View::Position p)
{
    int beginX = 0, beginY = 0, dx = 0, dy = 0, textX = 0, textY = 0;
    switch(p)
    {
    case BOTTOM:
        beginX = windowSize - 3*cardWidth;
        beginY = windowSize - cardHeight;
        textX = windowSize - 4*cardWidth;
        textY = windowSize - cardHeight - windowSize/25;
        dx = -cardWidth;
        break;
    case LEFT:
        beginX = cardHeight;
        beginY = windowSize - 3*cardWidth;
        textX = cardHeight;
        textY = windowSize - 4*cardWidth;
        dy = -cardWidth;
        break;
    case TOP:
        beginX = 3*cardWidth;
        beginY = 0;
        textX = 4*cardWidth;
        textY = cardHeight;
        dx = cardWidth;
        break;
    case RIGHT:
        beginX = windowSize - cardHeight;
        beginY = 3*cardWidth;
        textX = windowSize - cardHeight - windowSize/25;
        textY = 4*cardWidth;
        dy = cardWidth;
    }

    for(unsigned i = 0; i < ammount; i++)
    {
        drawCard(beginX - dx, beginY - dy, p, true);
        for(int j = 0; j < 3; j++)
        {
            drawCard(beginX + j * dx, beginY + j * dy, p, false);
            if(j == 2)
                drawLabel(textX, textY);
        }
        beginX += 5*dx;
        beginY += 5*dy;
        textX += 5*dx;
        textY += 5*dy;
    }
}

void View::drawCards()
{
    int i = 0;
    int players = 12;
    while(players > 0)
    {
        drawCardsLine(3, static_cast<Position>(i));
        i++;
        players -= 3;
    }
}

void View::drawButtons()
{
    if(user == PLAYER)
    {
        Button *submitButton = new Button(windowSize/2 - 100, windowSize/2, "Submit");
        QObject::connect(submitButton, SIGNAL(clicked()), this, SLOT(passCard()));
        scene->addItem(submitButton);
        Button *exchangeButton = new Button(windowSize/2, windowSize/2, "Exchange");
        connect(exchangeButton, SIGNAL(clicked()), this, SLOT(exchangeCard()));
        scene->addItem(exchangeButton);
    }
}

void View::setPlayerCards()
{
    unsigned n = game->getCurrentPlayerIndex();
    for(unsigned i = 0; i < 3; i++)
    {
        playerCards.insert(playerCards.begin(), cards[n * 3 + i]);
    }
    playerCards.insert(playerCards.begin(), floatingCards.at(n));
}

void View::activatePlayerCards()
{
    for(CardView *c : playerCards)
        c->setCardActive(true);
}

void View::deactivatePlayerCards()
{
    for(CardView *c : playerCards)
        c->setCardActive(false);
}

void View::initialize()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();
    if(screenWidth/2 > screenHeight)
        windowSize = screenHeight*3/4;
    else
        windowSize = screenWidth/2;
    cardWidth = windowSize/18;
    cardHeight = windowSize/10;
    scaleFactor = static_cast<qreal>(cardWidth)/static_cast<qreal>(imageWidth);

    scene = new QGraphicsScene();
    scene->setSceneRect(0, 0, windowSize, windowSize);
    setScene(scene);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFixedSize(windowSize, windowSize);
    setGeometry(0, 0, windowSize, windowSize);
    setFrameShape(QGraphicsView::NoFrame);

    drawCards();
    drawButtons();
    show();
}

void View::update(std::vector<Card> c, int player, Card floatingCard)
{
    unsigned n = game->getPlayers().size() * 3;
    for(unsigned i = 0; i < n; i++)
        cards[i]->setVisible(true);

    unsigned i = 0;
    for(Card card : c)
    {
        cards[i]->update(card, user);
        i++;
    }

    i = 0;
    std::vector<int> ids = game->getPlayersIds();
    for(int id : ids)
    {
        labels[i++]->setPlainText(QString::number(id));
    }

    if(player == 0)
    {
        floatingCards[game->getPlayers().size()-1]->setVisible(false);
        floatingCards[player]->update(floatingCard, user);
        floatingCards[player]->setVisible(true);
    }
    else
    {
        floatingCards[player-1]->setVisible(false);
        floatingCards[player]->update(floatingCard, user);
        floatingCards[player]->setVisible(true);
    }
    if(user == PLAYER)
        for(CardView *card : playerCards)
            card->update(card->getCard(), OBSERVER);
    if(user == PLAYER)
        if(game->getCurrentPlayerId() == game->getPlayerId())
            activatePlayerCards();
        else
            deactivatePlayerCards();
}

void View::endGame()
{
    scene->clear();
    QMessageBox msg;
    msg.setText("Game has ended");
    msg.exec();
}

View::View()
{
    initialize();
}

View::View(Game *g, unsigned u)
{
    user = static_cast<User>(u);
    initialize();
    game = g;
    if(user == PLAYER)
        setPlayerCards();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timer_update()));
    timer->start(100);
}

void View::timer_update()
{
    if(game->getIsStarted())
        update();
}
View::~View()
{

}

void View::setUser(unsigned u)
{
    user = User(u);
}

void View::update()
{
    if(game->getIsFinished())
        endGame();
    update(game->getPlayersCards(), game->getCurrentPlayerIndex(), game->getFloatingCard());
}

void View::passCard()
{
    for(CardView *c : playerCards)
        if(c->getIsSelected())
        {
            game->passCard(c->getCard());
            deactivatePlayerCards();
            break;
        }
}

void View::exchangeCard()
{
    if(game->getCurrentPlayer()->getCanExchange())
    {
        game->exchangeCard();
    }
}

void View::cardClicked()
{
    for(CardView *c : playerCards)
        if(c->getIsSelected())
        {
            c->setIsSelected(false);
            c->moveBy(0, 10);
            break;
        }
}
