#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QDataStream>
#include <QTcpSocket>
#include <QTextEdit>

class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void sendMsg();
    void updMsgs();
    void openConnection();
    void displayError(QAbstractSocket::SocketError socketError);
    void closeSocket();
    void disconnectClient();
    void enableButtons();

private:
    QLabel *helpLabel = nullptr;
    QComboBox *hostCombo = nullptr;
    QLineEdit *portLineEdit = nullptr;

    QPushButton *connectButton = nullptr;
    QPushButton *sendButton = nullptr;
    QPushButton *disconnectButton = nullptr;
    QTextEdit *chatArea = nullptr;
    QLineEdit *msgArea = nullptr;
    QTcpSocket *tcpSocket = nullptr;
    QDataStream in;
    QString currentMsg;
};

#endif // WIDGET_H
