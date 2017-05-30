#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QTextStream>
#include <QtGui>


std::list<QString> MainWindow::chat;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),

    ui(new Ui::MainWindow){
    ui->setupUi(this);
    ui->textBrowser->setText("Chose your nickname: ");

    this->setFixedSize(290, 550);
    this->userName = this->messageToSend = "";
    this->currentTime = QDateTime::currentDateTime().time().toString();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(setText()));
    timer->start(100);

}

MainWindow::~MainWindow()
{
    delete ui;
    delete timer;
}

void MainWindow::on_pushButton_clicked()
{
    if(this->userName == "")
    {
        getUserNickname();
    }
    else
    {
        setMessage(ui->textEdit->toPlainText());
        ui->textEdit->clear();
        char* message = prepareMessage();
        sendToPipe(message);
    }
}

char* MainWindow::prepareMessage()
{
    QString tmp = "";
    setTime(QDateTime::currentDateTime().time().toString());
    tmp = this->userName + "(" + this->currentTime + "): " + this->messageToSend;

    return qstrdup(qPrintable(tmp));
}

void MainWindow::sendToPipe(const char* message)
{
    //serialize();
    std::cout << message << std::endl;
}

void MainWindow::readFromPipe(QString message)
{

    //QString message = deserialize(message);
    if(message.size() <= MESSAGELENGTH)
    {
        chat.push_back(message.trimmed());
    }
    else
    {
        std::cout << "ERROR: MESSAGE TO LONG - MAX MESSAGE SIZE: 50";
    }
}

void MainWindow::getUserNickname()
{
    QString tmp = ui->textEdit->toPlainText();
    if(checkUserName(tmp))
    {
        srand(time(NULL));
        std::string s = std::to_string(rand() % 255);
        setUser(tmp + QString::fromStdString(s));
        setUser(this->userName.replace("\n","").replace("\t",""));
        ui->textBrowser->clear();
        ui->textEdit->clear();
        ui->textBrowser->setTextColor(QColor(0, 0, 0));
        chat.push_back("Your username is: " + this->userName);
    }
    else{
        this->userName = "";
        ui->textBrowser->setTextColor(QColor(255,0,0));
        ui->textBrowser->append("Incorrect user name - length must be between 0 - 16 characters (whitespace doesnt count). Try again:");
        ui->textEdit->clear();
    }
}

bool MainWindow::checkUserName(QString nick)
{
    if(nick.length() <= 16 && nick.length() > 0 && nick.trimmed() != "")
        return true;
    return false;
}

void MainWindow::setText()
{
    if(this->userName != ""){
        ui->textBrowser->clear();
        if(chat.size() > MAX_SIZE - 1)
        {
                chat.pop_front();
        }
        for(auto it = chat.begin(); it != chat.end(); it++)
        {
            ui->textBrowser->append(*it);
        }
    }
}
void MainWindow::setUser(QString usr)
{
    this->userName = usr;
}
void MainWindow::setMessage(QString msg)
{
    this->messageToSend = msg;
}
void MainWindow::setTime(QString time)
{
    this->currentTime = time;
}
QString MainWindow::getUser()
{
    return this->userName;
}
QString MainWindow::getMessage()
{
    return this->messageToSend;
}
QString MainWindow::getTime()
{
    return this->currentTime;
}
Ui::MainWindow* MainWindow::getUi(){
    return this->ui;
}
