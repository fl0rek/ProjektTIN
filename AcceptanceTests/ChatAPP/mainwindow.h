#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define MAX_SIZE 24
#define MESSAGELENGTH 77

#include <serial.h>
#include <QMainWindow>
#include <QDateTime>
#include <list>
#include <sstream>
#include <pthread.h>
#include <signal.h>
#include <fstream>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow( pthread_t process,QWidget *parent = 0);
    ~MainWindow();
    char* prepareMessage(QString);
    void sendToPipe(Message msg);
    static void readFromPipe(std::string);
    void getUserNickname();
    bool checkUserName(QString);

    std::string serialize(Message msg);
    Message deserialize(std::string s);

private slots:

    void on_pushButton_clicked();
    void setText();
    void acceptTest();

private:

    Ui::MainWindow *ui;

    QTimer *timerSender;
    QString userName;
    QString currentTime;
    QString messageToSend;
    pthread_t reader;
    std::ofstream of;
    std::ofstream rcv;
    static std::list<std::string> chat;
    int8_t counter;

};

#endif // MAINWINDOW_H
