#include "Model/game.h"
#include "View/view.h"
#include "debug.h"

#include <QApplication>
#include <iostream>
#include <pthread.h>
#include <unistd.h>

/*
 * author Adrian Sobolewski
 *
 * main gets provided args to launch a thread that stats the game and to launch another thread listening for messages
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
    if(user == 2)
        g = new Game(1);
    else
        g = new Game(0);
    if(user != 2)
        v = new View(g, user);
    a.exec();
    pthread_exit(0);
}


void *reader(void *)
{
    unsigned char message[256];
    while(1)
    {
        int x = read(STDIN_FILENO, message, 256);
        Tlv buffer(message, x);

        if(buffer.isTagPresent(tag::game_tags::terminate))
            break;
        if(buffer.isTagPresent(tag::game_tags::invalid_step))
            continue;
        g->acceptMessage(buffer);
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
            if(user < 0 || user > 2)
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

    delete v;
    delete g;

    return 0;
}
