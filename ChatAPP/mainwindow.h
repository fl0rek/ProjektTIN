#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define MAX_SIZE 24
#define MESSAGELENGTH 77

#include <serial.h>
#include <QMainWindow>
#include <QDateTime>
#include <list>
#include <sstream>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
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

private:

    Ui::MainWindow *ui;
    QTimer *timer;

    QString userName;
    QString currentTime;
    QString messageToSend;


    static std::list<std::string> chat;

};

#endif // MAINWINDOW_H
