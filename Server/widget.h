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
    void sendMsgs();
    void sendMsgs_all();
    void hanleNewConnection();
    void hanleReadyRead();
    void dropClient();

private:
    QLabel *statusLabel = nullptr;
    QTcpServer *tcpServer = nullptr;
    QVector<QString> msgs;

    QVector<QTcpSocket*> clients;

    QDataStream in;
};

#endif // WIDGET_H
