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
                            "Now you can launch client tools.")
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
    msgs << "Server started!";
    senderName.push_back("%con%");

    in.setVersion(QDataStream::Qt_4_0);
}

Widget::~Widget()
{
    addMsg("Server died :(", "%con%");
    addMsg("--------------- <<", "%con%");

    sendMsgs_all();
    msgs.clear();
    senderName.clear();

    clients.clear();
    clientsNames.clear();
    for (int i = 0; i < clients.size(); i++)
        dropClient(clients.at(i));
}

void Widget::addMsg(QString msg, QString sender){
    msgs.append(msg);
    senderName.append(sender);
}

/*void Widget::sendMsgs(QTcpSocket *clientConnection)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << msgs.size();
    clientConnection->write(block);
    clientConnection->flush();
    block.clear();

    for (int i = 0; i < msgs.size(); i++) //Запись всех сообщений в блок
        out << msgs.at(i);

    clientConnection->write(block);
    clientConnection->flush();
}*/

void Widget::sendMsgs_all()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    QString msgs_t = "";

    for (int i = 0; i < msgs.size(); i++){ //Заливаем все сообщения в строку
        msgs_t.append((senderName.at(i) == "%con%" ?
                           ">> " :
                           "|" + (senderName.at(i) + "| > ")) +
                      msgs.at(i) + "\n");
    }

    out << msgs_t;

    for (int i = 0; i < clients.size(); i++){ //Отправка всем клиентам сообщения
        clients.at(i)->write(block);
        clients.at(i)->flush();
    }
}

void Widget::hanleNewConnection()
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    in.setDevice(clientConnection);
    connect(clientConnection, &QAbstractSocket::readyRead, this, &Widget::hanleReadyRead);
    clients.append(clientConnection);
    enteredNick.append(false);
    clientsNames.append("User #" + QVariant(clients.size()).toString());
    connect(clientConnection, SIGNAL(disconnected()), this, SLOT(dropClient()));
    sendMsgs_all();
}

void Widget::hanleReadyRead()
{
    QString msg;

    int userNum = -1; //Определяем id отправителя
    for (int i = 0; i < clients.size(); i++){
        if (dynamic_cast<QTcpSocket*>(sender()) == clients.at(i))
            userNum = i;
    }

    // Read msg from client
    in.setDevice(dynamic_cast<QTcpSocket*>(sender())); //Фишка мультиюзеров тут
    in.startTransaction();
    in >> msg;
    if (!in.commitTransaction() || msg.isEmpty())
        return;
    //qDebug() << "Msg: " << msg;
    if (enteredNick.at(userNum)){
        if (clientsNames.at(userNum) == "%con%" //Kick ability for admin
                && msg.contains("!kick")){
            QString aim = msg.mid(6, msg.length() - 1);
            for (int i = 0; i < clientsNames.size(); i++){
                if (clientsNames.at(i) == aim){
                    dropClient(clients.at(i));
                    break;
                }
            }
            return;
        }
        msgs.push_back(msg);
        senderName.append(clientsNames.at(userNum));
    }
    else{
        clientsNames.replace(userNum, msg);
        enteredNick.replace(userNum, true);
        if (clientsNames.last() != "%con%") //Не детектим коннект админа)
            addMsg(clientsNames.at(userNum) + " connected!", "%con%");
    }
    while(msgs.size() > 50) //Не более 50 сообщений
        msgs.pop_front();

    sendMsgs_all();
}

void Widget::dropClient(QTcpSocket *client)
{
    for (int i = 0; i < clients.size(); i++)
        if (clients.at(i) == client){
            if (clientsNames.at(i) != "%con%"){
                msgs.append(clientsNames.at(i) + " left us...");
                senderName.append("%con%");
                sendMsgs_all();
            }
            clients.removeAt(i);
            clientsNames.removeAt(i);
            enteredNick.removeAt(i);
        }
    disconnect(client, &QAbstractSocket::readyRead, this, &Widget::hanleReadyRead);
    client->disconnectFromHost();
}
void Widget::dropClient()
{
    QTcpSocket *client = dynamic_cast<QTcpSocket*>(sender());
    dropClient(client);
}
