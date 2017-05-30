#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define MAX_SIZE 24
#define MESSAGELENGTH 77

#include <QMainWindow>
#include <QDateTime>
#include <list>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief MainWindow
     *          Initialize chat prompt message, set its size, set fixed size, start timer
     * @param parent
    */
    explicit MainWindow(QWidget *parent = 0);

    /**
      * @brief ~MainWindow
      *         Destroy ui layout and QTimer
    */
    ~MainWindow();

    /**
     * @brief prepareMessage
     *          Add username and current time message
     * @return
     *          Converted message from QString to char*
     */
    char* prepareMessage();

    /**
     * @brief sendToPipe
     *          Send message to STDOUT_FILENO for either client or server
     * @param msg
     *          Prepared message in correct format
     */
    void sendToPipe(const char* msg);

    /**
     * @brief readFromPipe
     *          Static function used by another thread to manipulate caht buffor
     * @param msg
     *          Message recived from another process (client/server) converted to QString
     */
    static void readFromPipe(QString msg);

    /**
     * @brief getUserNickname
     *          Function resposible for getting getting correct username from chat user. If user provides
     *          incorrect name funcition displays communicate and asks for correct name. In other case
     *          user sees the message from application and joins the chat.
     */
    void getUserNickname();

    /**
     * @brief checkUserName
     *          Function resposible for checking username. Username must be shorter than 16 chars,
     *          cannot be empty. White spaces are omitted.
     * @param msg
     *          username from textEdit
     *
     */
    bool checkUserName(QString msg);

    /**
     * @brief serialize
     */
    QByteArray serialize();

    /**
     * @brief deserialize
     * @param msg
     */
    void deserialize(QString msg);

    void setUser(QString usr);
    void setTime(QString time);
    void setMessage(QString msg);
    QString getUser();
    QString getTime();
    QString getMessage();
    Ui::MainWindow* getUi();
    static std::list<QString> chat;
private slots:

    /**
     * @brief on_pushButton_clicked
     *          Slot responsible for actions when "send" button is pushed
     */
    void on_pushButton_clicked();

    /**
     * @brief setText
     *          Slot responsible for displaying text on textBrowser
     */
    void setText();

private:

    Ui::MainWindow *ui;
    QTimer *timer;

    QString userName;
    QString currentTime;
    QString messageToSend;




};

#endif // MAINWINDOW_H
