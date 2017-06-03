#ifndef CARDVIEW_H
#define CARDVIEW_H
#include "Model/card.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QString>


/*
 * author Adrian Sobolewski
 *
 * CardView represents a custom QObject - a Pixmap with Card data inside
 */
class CardView: public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
    Card card;
    bool isSelected = false;
    bool cardActive = false;
public:
    /**
     * @brief CardView
     *      creates a CardView and sets it's Pixmap to a default card back image
     * @param parent
     *      parent QGraphicsView
     */
    CardView(QGraphicsItem *parent=NULL);
    void mousePressEvent(QGraphicsSceneMouseEvent *);
    /**
     * @brief update
     *      updates the Pixmap depending on the given Card data and user
     * @param c
     *      Card data
     * @param u
     *      user:   0 = observer that can see all cards
     *              1 = player that can see only own cards
     */
    void update(Card c, unsigned u);

    Card getCard();
    void setCardActive(bool b);
    void setIsSelected(bool b);
    bool getIsSelected();
signals:
    void clicked();
};

#endif // CARDVIEW_H
