#include "button.h"

#include <QBrush>
#include <QDebug>

Button::Button(int x, int y, QString s, QGraphicsItem *parent): QGraphicsRectItem(parent)
{
    setRect(x, y, 100, 30);
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(Qt::gray);
    setBrush(brush);

    text = new QGraphicsTextItem(s, this);
    text->setPos(x + rect().width()/2 - text->boundingRect().width()/2, y + rect().height()/2 - text->boundingRect().height()/2);
    setAcceptHoverEvents(true);
}

//to check - not sure if this works without parameter name
void Button::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    emit clicked();
}

void Button::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(Qt::darkGray);
    setBrush(brush);
}

void Button::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(Qt::gray);
    setBrush(brush);
}
