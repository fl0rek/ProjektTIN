#ifndef BUTTON_H
#define BUTTON_H
#include "view.h"

#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QString>

/*
 * author Adrian Sobolewski
 *
 *  Button represents a custor QObject - a Rectangle with text inside
 */

class Button: public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    /**
     * @brief Button
     *      creates a 100px x 30px rectangle on a given position with a given text
     * @param x
     *      x coord
     * @param y
     *      y coord
     * @param s
     *      text in the rectangle
     * @param parent
     *      parent QGraphicsView
     */
    Button(int x, int y, QString s, QGraphicsItem *parent=NULL);
    void mousePressEvent(QGraphicsSceneMouseEvent *);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
signals:
    void clicked();
private:
    QGraphicsTextItem *text;
};

#endif // BUTTON_H
