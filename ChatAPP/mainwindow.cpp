#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QTextStream>
#include <QtGui>


std::list<std::string> MainWindow::chat;
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
        this->messageToSend = ui->textEdit->toPlainText();
        ui->textEdit->clear();
        this->currentTime = QDateTime::currentDateTime().time().toString();
        Message msg;
        msg.user= this->userName.toStdString();
        msg.time = this->currentTime.toStdString();
        msg.m = this->messageToSend.toStdString();
        sendToPipe(msg);
    }
}

char* MainWindow::prepareMessage(QString msg)
{
    QString tmp = "";
    this->currentTime = QDateTime::currentDateTime().time().toString();
    tmp = this->userName + "(" + this->currentTime + "): " + msg;

    return qstrdup(qPrintable(tmp));
}

void MainWindow::sendToPipe(Message msg)
{
    std::cout << serialize(msg) <<std::endl;
}

void MainWindow::readFromPipe(std::string message)
{
    if(message.size() <= 250)
    {
        chat.push_back(message);
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
        this->userName = tmp + QString::fromStdString(s);
        ui->textBrowser->clear();
        ui->textEdit->clear();
        ui->textBrowser->setTextColor(QColor(0, 0, 0));
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
    if(this->userName != "" ){
        ui->textBrowser->clear();
        if(chat.size() > MAX_SIZE - 1)
        {
                chat.pop_front();
        }
        for(auto it = chat.begin(); it != chat.end(); it++)
        {
            Message msg = deserialize(*it);
            QString toDisplay = QString::fromStdString(msg.user) + "(" +QString::fromStdString(msg.time) + "): "+
                    QString::fromStdString(msg.m);
            ui->textBrowser->append(toDisplay);

        }
    }
}
std::string MainWindow::serialize(Message msg)
{
    std::stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << msg;
    return ss.str();

}
Message MainWindow::deserialize(std::string s)
{
    Message msg;
    std::stringstream ss;
    ss << s;
    boost::archive::text_iarchive ia(ss);
    ia >> msg;
    return msg;
}
