#include "client.h"
#include <QDebug>

client::client(QObject* parent)
    : QObject(parent),
    m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected, this, &client::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &client::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &client::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &client::onErrorOccurred);
}

client::~client()
{
    disconnectFromServer();
}

void client::connectToServer(const QString& host, quint16 port)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        qDebug() << "[client] Already connected.";
        return;
    }

    qDebug() << "[client] Connecting to" << host << "port" << port;
    m_socket->connectToHost(host, port);

    connect(m_socket, &QTcpSocket::connected, []() {
        qDebug() << "[client] Successfully connected to server.";
    });
    connect(m_socket, &QTcpSocket::errorOccurred, [](QAbstractSocket::SocketError err) {
        qDebug() << "[client] Connection error:" << err;
    });
}

void client::disconnectFromServer()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }
}

void client::sendCommand(const QString& command)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(command.toUtf8() + '\n');
    }else {
        qDebug() << "[client] Cannot send command, socket not connected.";
    }

}

void client::login(const QString& username, const QString& password)
{
    qDebug() << "[client] login() called with username:" << username;
    sendCommand("LOGIN " + username + " " + password);
}

void client::registerUser(const QString& username, const QString& password)
{
    sendCommand("REGISTER " + username + " " + password);
}

void client::sendMessage(const QString& message)
{
    sendCommand("MSG " + message);
}

void client::sendPrivateMessage(const QString& recipient, const QString& message)
{
    sendCommand("PMSG " + recipient + " " + message);
}

void client::requestUserList()
{
    sendCommand("USERLIST");
}

void client::onConnected()
{
    qDebug() << "Connected to server.";
    emit connected();
}

void client::onDisconnected()
{
    qDebug() << "Disconnected from server.";
    emit disconnected();
}
void client::requestHistory()
{
    sendCommand("HISTORY");
}

void client::onReadyRead()
{
    while (m_socket->canReadLine()) {
        QString line = QString::fromUtf8(m_socket->readLine()).trimmed();
       qDebug()<< "Server says:" << line;

        QStringList parts = line.split(' ');

        if (parts[0] == "LOGIN_OK" && parts.size() >= 4) {
            bool ok;
            int userId = parts[1].toInt(&ok);
            QString username = parts[2];
            bool isAdmin = (parts[3] == "1");
            emit loginResult(true, userId, username, isAdmin);
        }
        else if (parts[0] == "LOGIN_FAIL") {
            emit loginResult(false, -1, "", false);
        }
        else if (parts[0] == "REGISTER_OK" && parts.size() >= 3) {
            bool ok;
            int userId = parts[1].toInt(&ok);
            if (ok) emit registerResult(true, userId, parts[2]);
        }
        else if (parts[0] == "REGISTER_FAIL") {
            emit registerResult(false, -1, "");
        }
        else if (parts[0] == "MSG" && parts.size() >= 3) {
            QString from = parts[1];
            QString text = parts.mid(2).join(" ");
            emit messageReceived(from, text);
        }
        else if (parts[0] == "PMSG" && parts.size() >= 4) {
            QString from = parts[1];
            QString to = parts[2];
            QString text = parts.mid(3).join(" ");
           qDebug() << "[CLIENT] Got PMSG from" << from << "to" << to << ":" << text;
            emit privateMessageReceived(from, to, text);
        }
        else if (parts[0] == "USERLIST" && parts.size() >= 2) {
            QStringList users = parts.mid(1);
            emit userListReceived(users);
        }
        else if (parts[0] == "HISTORY_MSG" && parts.size() >= 4) {
            QString sender = parts[1];
            QString time = parts[2];
            QString text = parts.mid(3).join(" ");
            emit messageReceived(sender, "[" + time + "] " + text);
        }
        else if (parts[0] == "HISTORY_PMSG" && parts.size() >= 5) {
            QString sender = parts[1];
            QString receiver = parts[2];
            QString time = parts[3];
            QString text = parts.mid(4).join(" ");
            emit privateMessageReceived(sender, receiver, "[" + time + "] " + text);
        }
        else if (parts[0] == "SERVER") {
            emit messageReceived("SERVER", parts.mid(1).join(" "));
        }
        else {
            emit messageReceived("SERVER", line);
        }
    }
}

void client::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QString err = m_socket->errorString();
    qDebug() << "Socket error:" << err;
    emit errorOccurred(err);
}
