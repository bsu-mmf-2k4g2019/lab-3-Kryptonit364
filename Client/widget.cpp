#include "widget.h"

#include <QHostInfo>
#include <QNetworkInterface>
#include <QGridLayout>
#include <QMessageBox>
#include <QTimer>
#include <QTextEdit>
#include <QScrollBar>
#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent),
    helpLabel(new QLabel("Name:")),
    hostCombo(new QComboBox),
    portLineEdit(new QLineEdit),
    connectButton(new QPushButton("Connect")),
    sendButton(new QPushButton("Send")),
    disconnectButton(new QPushButton("Disconnect")),
    chatArea(new QTextEdit),
    msgArea(new QLineEdit),
    tcpSocket(new QTcpSocket(this))
{
    chatArea->setReadOnly(1);
    hostCombo->setEditable(true);
    sendButton->setEnabled(0);
    msgArea->setReadOnly(1);
    // find out name of this machine
    QString name = QHostInfo::localHostName();
    if (!name.isEmpty()) {
        hostCombo->addItem(name);
        QString domain = QHostInfo::localDomainName();
        if (!domain.isEmpty())
            hostCombo->addItem(name + QChar('.') + domain);
    }
    if (name != QLatin1String("localhost"))
        hostCombo->addItem(QString("localhost"));
    // find out IP addresses of this machine
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // add non-localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (!ipAddressesList.at(i).isLoopback())
            hostCombo->addItem(ipAddressesList.at(i).toString());
    }
    // add localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i).isLoopback())
            hostCombo->addItem(ipAddressesList.at(i).toString());
    }

    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    auto hostLabel = new QLabel("Server name:");
    hostLabel->setBuddy(hostCombo);
    auto portLabel = new QLabel("Server port:");
    portLabel->setBuddy(portLineEdit);
    helpLabel->setBuddy(msgArea);

    msgArea->setText("Connect to some server may be :?");
    connectButton->setDefault(1);
    disconnectButton->setEnabled(0);

    in.setDevice(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    connect(hostCombo, &QComboBox::editTextChanged,
            this, &Widget::enableButtons);
    connect(portLineEdit, &QLineEdit::textChanged,
            this, &Widget::enableButtons);
    connect(connectButton, SIGNAL(clicked()),
            this, SLOT(openConnection()));
    connect(sendButton, SIGNAL(clicked()),
            this, SLOT(sendMsg()));
    connect(msgArea, &QLineEdit::textChanged, this, &Widget::enableButtons);
    connect(disconnectButton, SIGNAL(clicked()),
            this, SLOT(disconnectClient()));
    //connect(tcpSocket, &QTcpSocket::disconnected, this, &Widget::disonnectClient);
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostCombo, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(connectButton, 2, 0, 1, 2);
    mainLayout->addWidget(chatArea, 3, 0, 5, 2);
    mainLayout->addWidget(helpLabel, 8, 0);
    mainLayout->addWidget(msgArea, 8, 1);
    mainLayout->addWidget(disconnectButton, 9, 0, 1, 1);
    mainLayout->addWidget(sendButton, 9, 1, 1, 1);

    portLineEdit->setFocus();

    enableButtons();
}
Widget::~Widget()
{

}
void Widget::openConnection()
{
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(updMsgs()));
    connect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
          this, &Widget::displayError);
    sendButton->setEnabled(false);
    disconnectButton->setEnabled(false);
    tcpSocket->abort();
    tcpSocket->connectToHost(hostCombo->currentText(),
                             portLineEdit->text().toInt());
    disconnectButton->setEnabled(1);
    sendButton->setEnabled(1);
    msgArea->clear(); msgArea->setReadOnly(0);

    hostCombo->setEnabled(0);
    portLineEdit->setEnabled(0);
    connectButton->setEnabled(0);
    connect(tcpSocket, &QAbstractSocket::readyRead,
            this, &Widget::updMsgs);
}
void Widget::updMsgs(){
    QString msgs_t;

    in.startTransaction();
    in >> msgs_t;

    if (!in.commitTransaction())
        return;

    chatArea->setText(msgs_t);
    QScrollBar *sb = chatArea->verticalScrollBar();
    sb->setValue(sb->maximum());
}
void Widget::sendMsg()
{
    helpLabel->setText("Message:");
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);

    out << msgArea->text();

    tcpSocket->write(block);
    tcpSocket->flush();
    msgArea->clear();
}
void Widget::disconnectClient(){
    msgArea->setText("Connect to some server may be :?");
    msgArea->setReadOnly(1);
    disconnect(tcpSocket, SIGNAL(connected()), this, SLOT(updMsgs()));
    disconnect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
          this, &Widget::displayError);

    tcpSocket->abort();
    sendButton->setEnabled(0);
    disconnectButton->setEnabled(0);

    hostCombo->setEnabled(1);
    portLineEdit->setEnabled(1);
    connectButton->setEnabled(1);
    chatArea->clear();
    msgArea->clear();
    helpLabel->setText("Name:");
}
void Widget::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Client"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the chat server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        disconnectClient();
        break;
    default:
        QMessageBox::information(this, tr("Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }

    sendButton->setEnabled(true);
    disconnectButton->setEnabled(true);
}
void Widget::enableButtons()
{
    connectButton->setEnabled(!hostCombo->currentText().isEmpty() &&
                                 !portLineEdit->text().isEmpty() &&
                              chatArea->toPlainText().isEmpty());
    sendButton->setEnabled(!msgArea->text().isEmpty());
}
