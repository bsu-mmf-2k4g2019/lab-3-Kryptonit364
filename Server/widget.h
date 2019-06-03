#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QVector>
#include <QDataStream>
#include <QTcpServer>

class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void sendMsgs_all();
    void hanleNewConnection();
    void hanleReadyRead();
    void dropClient(QTcpSocket*);
    void dropClient();

private:
    void addMsg(QString msg, QString sender);
    QLabel *statusLabel = nullptr;
    QTcpServer *tcpServer = nullptr;
    QVector<QString> msgs;
    QVector<QString> senderName;

    QVector<QTcpSocket*> clients;
    QVector<bool> enteredNick;
    QVector<QString> clientsNames;

    QDataStream in;
};

#endif // WIDGET_H
