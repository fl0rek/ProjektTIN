#include "Model/game.h"
#include "View/view.h"
#include "debug.h"

#include <QApplication>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <QDebug>


/* TODO
 * check game->isValid()
 * game-server communication
 * game-client communication
 *
 * test if view is working properly with message sending
 * documentation
 * code refactor
 */

Game *g;
View *v;
int user = -1;

struct threadData
{
    int argc;
    char *argv;
};


void *gameApp(void *ptr)
{
    struct threadData *data;
    data = (struct threadData*)ptr;
    int argc = data->argc;
    char *argv = data->argv;

    QApplication a(argc, &argv);
    a.setApplicationName("Game");
    if(user == 1)
       g = new Game(std::string(data->argv));
    else
       g = new Game();
    if(user != 2)
        v = new View(g, user);
    a.exec();

    pthread_exit(0);
}


void *reader(void *)
{
    char message[256];
    while(1)
    {
        int x = read(STDIN_FILENO, message, 256);
        std::string msg(message, x);
        msg = trim(msg);
        //TODO game-server game-client proper communication
        if(msg != "")
        {
            g->acceptMessage(msg);
            if(user != 2)
                v->update();
        }
        for(unsigned i = 0; i < msg.size(); i++)
            message[i] = ' ';
    }
    pthread_exit(0);
}


int main(int argc, char *argv[])
{
    if(argc > 1)
        user = static_cast<int>(*argv[1]) - 48;
    switch(argc)
    {
        case 1:
            std::cout<<invalidArgs;
            exit(1);
            break;
        case 2:
            if(user < 0 && user > 2)
            {
                std::cout<<invalidArgs;
                exit(1);
            }
            break;
        case 3:
            if(user != 1)
            {
                std::cout<<invalidArgs;
                exit(1);
            }
            break;
        default:
            std::cout<<invalidArgs;
            exit(1);
        break;
    }

    struct threadData data[1];
    pthread_t game, rd;

    data[0].argc = argc;
    data[1].argv = *argv;

    if(pthread_create(&game, NULL, gameApp, (void *) &data[0]))
        pthreadCreateError(errno);
    if(pthread_create(&rd, NULL, reader, NULL))
        pthreadCreateError(errno);

    if(pthread_join(rd, NULL))
        pthreadJoinError(errno);
    if(pthread_join(game, NULL))
        pthreadJoinError(errno);

    return 0;
}