#include "widget.h"

#include <QDebug>
#include <QTcpSocket>
#include <QNetworkInterface>

#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRandomGenerator>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    qDebug() << "Server constructor is called";
    statusLabel = new QLabel();
    statusLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen()) {
        qDebug() << "Unable to make server listen";
        statusLabel->setText(QString("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
        close();
        return;
    }

    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

    statusLabel->setText(QString("The server is running on\n\nIP: %1\nport: %2\n\n"
                            "Run the Fortune Client example now.")
                         .arg(ipAddress).arg(tcpServer->serverPort()));
    qDebug() << "Start server on: " << ipAddress << ":" << tcpServer->serverPort();

    auto quitButton = new QPushButton(tr("Quit"));
    connect(quitButton, &QAbstractButton::clicked, this, &QWidget::close);
    connect(tcpServer, &QTcpServer::newConnection, this, &Widget::hanleNewConnection);

    auto buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);
    buttonLayout->addStretch(1);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);

    // Initialize msgs
    msgs << "Welcome!";

    in.setVersion(QDataStream::Qt_4_0);
}

Widget::~Widget()
{

}

void Widget::sendMsgs()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket*>(sender());

    out << msgs.size();
    clientConnection->write(block);
    block.clear();

    for (int i = 0; i < msgs.size(); i++) //Запись всех сообщений в блок
        out << msgs.takeAt(i);

    clientConnection->write(block);
}

void Widget::sendMsgs_all()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << msgs.size();
    for (int i = 0; i < clients.size(); i++) //Отправка всем клиентам кол-ва сообщений
        clients.takeAt(i)->write(block);
    block.clear();

    for (int i = 0; i < msgs.size(); i++) //Запись всех сообщений в блок
        out << msgs.takeAt(i);

    for (int i = 0; i < clients.size(); i++) //Отправка всем клиентам сообщения
        clients.takeAt(i)->write(block);
}

void Widget::hanleNewConnection()
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    in.setDevice(clientConnection);
    connect(clientConnection, &QAbstractSocket::readyRead, this, &Widget::hanleReadyRead);
    clients.append(clientConnection);
    connect(clientConnection, SIGNAL(disconnect()), this, SLOT(dropClient()));

}

void Widget::hanleReadyRead()
{
    QString msg;

    // Read msg from client
    in.startTransaction();
    in >> msg;
    if (!in.commitTransaction())
        return;
    qDebug() << "Msg: " << msg;
    msgs.push_back(msg);

    while(msgs.size() > 50) //Не более 50 сообщений
        msgs.pop_front();

    sendMsgs_all();
    //dropClient(dynamic_cast<QTcpSocket*>(sender()));
}


void Widget::dropClient()
{
    QTcpSocket *client = dynamic_cast<QTcpSocket*>(sender());
    disconnect(client, &QAbstractSocket::readyRead, this, &Widget::hanleReadyRead);
    //connect(client, &QAbstractSocket::disconnected, client, &QObject::deleteLater);
    for (int i = 0; i < clients.size(); i++)
        if (clients.takeAt(i) == client)
            clients.removeAt(i);
    msgs.append("Someone left us...");
    client->disconnectFromHost();
}
