#include "server.h"
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <QHostAddress>

Server::Server(QObject* parent) : QTcpServer(parent) {}

bool Server::startServer(quint16 port)
{
    if (!listen(QHostAddress::Any, port))
    {
        qDebug() << "Server failed to start on port" << port;
        return false;
    }
    connect(this, &QTcpServer::newConnection, this, &Server::onNewConnection);
    qDebug()<< "Server started on port" << port;
    return true;
}

void Server::stopServer() {
    close();
    for (QTcpSocket* sock : m_clients) {
        sock->disconnectFromHost();
    }
    m_clients.clear();
    m_clientIds.clear();
    m_bannedIps.clear();
}

void Server::setDatabase(std::shared_ptr<Database> db)
{
    m_database = db;
}

void Server::onNewConnection()
{
    while (hasPendingConnections()){
        QTcpSocket*clientSocket = nextPendingConnection();
        QString ip = clientSocket->peerAddress().toString();
        if (m_bannedIps.contains(ip) && m_bannedIps[ip]) {
            qDebug() << "Rejected connection from banned IP:" << ip;
            clientSocket->disconnectFromHost();
            continue;
        }

        m_clients.append(clientSocket);

        connect (clientSocket, &QTcpSocket::readyRead, this, &Server::onReadyRead);
        connect (clientSocket, &QTcpSocket::disconnected, this, &Server::onClientDisconnect);

        qDebug() << "New client connected:" << ip;
    }
}

void Server::onReadyRead()
{
    auto *clientSocket = qobject_cast<QTcpSocket*> (sender());
    if (!clientSocket) return;

    while (clientSocket->canReadLine())
    {
        QString line = QString::fromUtf8(clientSocket->readLine()).trimmed();
        qDebug() << "Received:" <<line;
        processCommand(clientSocket, line);
    }
}

void Server::onClientDisconnect()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*> (sender());
    if (!clientSocket) return;

    m_clients.removeAll(clientSocket);
    m_clientIds.erase(clientSocket);
    clientSocket->deleteLater();
    qDebug()<< "Client disconnected";
}

void Server::processCommand(QTcpSocket* client, const QString& msg)
{
    QStringList parts = msg.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty()) return;

    QString cmd = parts[0].toUpper();

    if (cmd == "KICK" && parts.size() > 1) {
        QString userToKick = parts[1];
        for (QTcpSocket* sock : m_clients) {
            if (m_clientIds.count(sock) && m_database->getUserName(m_clientIds[sock]).c_str() == userToKick) {
                kickClient(sock);
                break;
            }
        }
    } else if (cmd == "BAN" && parts.size() > 1) {
        QString userToBan = parts[1];
        for (QTcpSocket* sock : m_clients) {
            if (m_clientIds.count(sock) && QString::fromStdString(m_database->getUserName(m_clientIds[sock])) == userToBan) {
                banClient(sock);
                break;
            }
        }
    }
    // Добавить обработку других команд
}

void Server::sendMessage(QTcpSocket* client, const QString& msg)
{
    if (!client) return;
    QByteArray data = msg.toUtf8();
    data.append('\n');
    client->write(data);
}

void Server::broadcast(const QString& msg)
{
    for (QTcpSocket* client : m_clients) {
        sendMessage(client, msg);
    }
}

void Server::kickClient(QTcpSocket* client)
{
    if (!client) return;
    qDebug() << "Kicking client:" << client->peerAddress().toString();
    sendMessage(client, "You have been kicked by server.");
    client->disconnectFromHost();
    m_clients.removeAll(client);
    m_clientIds.erase(client);
}

void Server::banClient(QTcpSocket* client)
{
    if (!client) return;
    QString ip = client->peerAddress().toString();
    qDebug() << "Banning client IP:" << ip;
    m_bannedIps[ip] = true;
    sendMessage(client, "You have been banned by server.");
    client->disconnectFromHost();
    m_clients.removeAll(client);
    m_clientIds.erase(client);
}

bool Server::banUserByName(const std::string& username)
{
    bool success = m_database->setUserBanStatus(username, true);
    if (!success) return false;

    for (QTcpSocket* sock : m_clients) {
        if (m_clientIds.count(sock) &&
            m_database->getUserName(m_clientIds[sock]) == username) {
            banClient(sock);
            break;
        }
    }
    return true;
}

bool Server::unbanUserByName(const std::string& username)
{
    return m_database->setUserBanStatus(username, false);
}

bool Server::kickUserByName(const std::string& username)
{
    for (QTcpSocket* sock : m_clients) {
        if (m_clientIds.count(sock) &&
            m_database->getUserName(m_clientIds[sock]) == username) {
            kickClient(sock);
            return true;
        }
    }
    return false;
}





