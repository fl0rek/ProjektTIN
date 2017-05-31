#include "cardview.h"
#include <QDebug>
#include <QKeyEvent>

CardView::CardView(QGraphicsItem *parent): QGraphicsPixmapItem(parent)
{
    setPixmap(QPixmap(":/images/Resources/Images/back.png"));
}

void CardView::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    if(cardActive)
    {
        if(!isSelected)
        {
            emit clicked();
            moveBy(0, -10);
            isSelected = true;
        }
        else
        {
            moveBy(0, 10);
            isSelected = false;
        }
    }
}

void CardView::update(Card c, unsigned u)
{
    card = c;
    QString s = ":/images/Resources/Images/";
    if(u == 0){
        switch(c.first)
        {
        case 0:
            s.append("a");
            break;
        case 1:
            s.append("2");
            break;
        case 2:
            s.append("3");
            break;
        case 3:
            s.append("4");
            break;
        case 4:
            s.append("5");
            break;
        case 5:
            s.append("6");
            break;
        case 6:
            s.append("7");
            break;
        case 7:
            s.append("8");
            break;
        case 8:
            s.append("9");
            break;
        case 9:
            s.append("10");
            break;
        case 10:
            s.append("j");
            break;
        case 11:
            s.append("q");
            break;
        case 12:
            s.append("k");
            break;
        default:
            s = ":/images/Resources/Images/blank";
            break;
        }

        switch(c.second)
        {
        case 0:
            s.append("h");
            break;
        case 1:
            s.append("d");
            break;
        case 2:
            s.append("c");
            break;
        case 3:
            s.append("s");
            break;
        default:
            s = ":/images/Resources/Images/";
            break;
        }
    }
    else
        s.append("back");
    s.append(".png");
    setPixmap(QPixmap(s));
}

Card CardView::getCard()
{
    return card;
}

void CardView::setCardActive(bool b)
{
   cardActive = b;
}

bool CardView::getIsSelected()
{
    return isSelected;
}

void CardView::setIsSelected(bool b)
{
    isSelected = b;
}

