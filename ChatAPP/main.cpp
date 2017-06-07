#include "mainwindow.h"
#include <QApplication>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <string>
#include <errno.h>
#include <debug.h>
#include <stdio.h>
#include "../libtlv/src/Tlv.cpp"

struct threadData{
    int arg1;
    char *arg2;
};
struct threadData dataToPass[1];

pthread_t rd, chat;
void* chatAPP(void* ptr){
    struct threadData *my_data;
    my_data = (struct threadData*) ptr;

    int argc = my_data->arg1;
    char* argv = my_data->arg2;

    QApplication a(argc, &argv);
    MainWindow w(rd);
    w.show();
    a.exec();

    pthread_exit(0);
}
void* reader(void* ptr){
    int x;
    while(1){
        if((x = read(STDIN_FILENO, ms, 550)) != -1){
        std::string msg(ms, x);
        MainWindow::readFromPipe(msg);
        }
        else
            break;
    }
    pthread_kill(chat, SIGKILL);
    pthread_exit(0);
}

int main(int argc, char *argv[]){
    if(argc != 1){
        printf("Incorrect input arguments.\n");
        exit(1);
    }

    dataToPass[0].arg1 = argc;
    dataToPass[1].arg2 = *argv;

    if(pthread_create(&rd, NULL, reader, NULL))
        pthreadCreateError(errno);

    if(pthread_create(&chat, NULL, chatAPP, (void *) &dataToPass[0]))
        pthreadCreateError(errno);

    if(pthread_join(rd, NULL))
        pthreadJoinError(errno);

    if(pthread_join(chat, NULL))
        pthreadJoinError(errno);

    return 0;
}
