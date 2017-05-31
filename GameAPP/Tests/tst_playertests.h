#ifndef TST_PLAYERTESTS_H
#define TST_PLAYERTESTS_H

#include <QString>
#include <QtTest/QtTest>
class PlayerTests : public QObject
{
    Q_OBJECT
public:
    PlayerTests();
private slots:
    void playerConstructor1();
    void playerConstructor2();
    void playerGiveCard();
    void playerRemoveCard();
    void playerHasCard();
    void playerHasThreeTrue();
    void playerHasThreeFalse();
};

#endif // TST_PLAYERTESTS_H
