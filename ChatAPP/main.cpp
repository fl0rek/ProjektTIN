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
/*
 * @author Szymon Bezpalko
 *
 * Chat application, after start application prompts for username - maximum size 16 chars, after choosing
 * nickname user can freely use chat. Message is send after pressing "send" button. Waiting for message is
 * realized by creating another tread which reads from STDIN_FILENO and adds the message to the chat buffor.
 * Sending message is realized simply by sending it to STDOUT_FILENO. Since communication between qt and non-qt
 * applications is not supported I used QTimer to to check if the change in buffer has occured.
 *
 */
struct threadData{
    int arg1;
    char *arg2;
};
struct threadData dataToPass[1];
/**
 * @brief chatAPP
 *          Thread responsible for running chat application.
 * @param ptr
 *          ptr - pointer to data which is required to run qt appplciations
 * @return
 *          0
 */
void* chatAPP(void* ptr){
    struct threadData *my_data;
    my_data = (struct threadData*) ptr;

    int argc = my_data->arg1;
    char* argv = my_data->arg2;

    QApplication a(argc, &argv);
    MainWindow w;
    w.show();
    a.exec();

    pthread_exit(0);
}
/**
 * @brief reader
 *          thread responsible for reading data from STDIN_FILENO and converting recived data to QString
 * @param ptr
 *          NULL
 * @return
 *          0
 */
void* reader(void* ptr){
    char ms[77];
    while(1){
        int x = read(STDIN_FILENO, ms, 77);
        std::string msg(ms, x);
        QString msm = QString::fromStdString(msg);
        if(msm.trimmed() != "")
            MainWindow::readFromPipe(msm);
        for(int i = 0; i < msm.size(); i++)
            ms[i] = ' ';
    }
    pthread_exit(0);
}

int main(int argc, char *argv[]){
    if(argc != 1){
        printf("Incorrect input arguments.\n");
        exit(1);
    }
    pthread_t rd, chat;

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
