#include "widget.h"

#include <QHostInfo>
#include <QNetworkInterface>
#include <QGridLayout>
#include <QMessageBox>
#include <QTimer>
#include <QTextEdit>
#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent),
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
    qDebug() << "Constructor is called";
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
    connect(disconnectButton, SIGNAL(clicked()),
            this, SLOT(disconnectClient()));

    connect(tcpSocket, SIGNAL(connected()), this, SLOT(updMsgs()));
    connect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
          this, &Widget::displayError);

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostCombo, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(connectButton, 2, 0, 1, 2);
    mainLayout->addWidget(chatArea, 3, 0, 5, 2);
    mainLayout->addWidget(msgArea, 8, 0, 1, 2);
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
    qDebug() << "Open connection is called";
    sendButton->setEnabled(false);
    disconnectButton->setEnabled(false);
    tcpSocket->abort();
    tcpSocket->connectToHost(hostCombo->currentText(),
                             portLineEdit->text().toInt());
    disconnectButton->setEnabled(1);
    sendButton->setEnabled(1);
    msgArea->clear(); msgArea->setReadOnly(0);

    hostCombo->setEnabled(0);
    portLineEdit->setReadOnly(1);
    connectButton->setEnabled(0);
}
void Widget::updMsgs(){
    chatArea->clear();

    in.startTransaction();

    int n;
    in >> n;

    if (!in.commitTransaction())
        return;

    QString *msgs = new QString[n];
    for (int i = 0; i < n; i++){
        in >> msgs[i];
        if (!in.commitTransaction())
            return;
        chatArea->append("> " + msgs[i] + "\n");
    }

    //disconnect(tcpSocket, &QAbstractSocket::readyRead, this, &Widget::readFortune);
}
void Widget::sendMsg()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);

    out << msgArea->text();

    tcpSocket->write(block);
    tcpSocket->flush();
}
void Widget::disconnectClient(){
    emit tcpSocket->disconnected();
    sendButton->setEnabled(0);
    disconnectButton->setEnabled(0);

    hostCombo->setEnabled(1);
    portLineEdit->setReadOnly(0);
    connectButton->setEnabled(1);
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
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
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
                                 !portLineEdit->text().isEmpty());

}
