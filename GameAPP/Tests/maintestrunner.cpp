#include <QString>
#include <QtTest>
#include "tst_playertests.h"
#include "tst_gametests.h"

int main(int argc, char *argv[])
{
    int status = 0;
    {
        PlayerTests pt;
        status |= QTest::qExec(&pt, argc, argv);
    }

    {
        GameTests gt;
        status |= QTest::qExec(&gt, argc, argv);
    }

   return status;
}

