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
    QMutexLocker locker(&m_clientsMutex);
    close();
    for (QTcpSocket* sock : m_clients) {
        sock->disconnectFromHost();
         sock->deleteLater();
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

        {
         QMutexLocker locker(&m_clientsMutex);
         m_clients.append(clientSocket);
        }
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

    QMutexLocker locker(&m_clientsMutex);
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

    if (cmd == "REGISTER" && parts.size() >= 3) {
        QString username = parts[1];
        QString password = parts[2];
        int id = m_database->addUser(username.toStdString(), password.toStdString());
        if (id > 0) {
            sendMessage(client, "REGISTER_OK " + QString::number(id) + " " + username);
        } else {
            sendMessage(client, "REGISTER_FAIL");
        }

    } else if (cmd == "LOGIN" && parts.size() >= 3) {
        QString username = parts[1];
        QString password = parts[2];
        int id = m_database->checkPassword(username.toStdString(), password.toStdString());

        if (id >= 0) {
            if (m_database->isUserBanned(username.toStdString())) {
                sendMessage(client, "LOGIN_FAILED_BANNED");
                client->disconnectFromHost();
                return;
            }

            {
                QMutexLocker locker(&m_clientsMutex);
                m_clientIds[client] = id;
            }
            bool isAdmin = m_database->isUserAdmin(username.toStdString());
            QString adminFlag = isAdmin ? "1" : "0";

            sendMessage(client, "LOGIN_OK " + QString::number(id) + " " + username + " " + adminFlag);

            // общий чат
            std::vector<std::string> publicMessages = m_database->getChatMessages();
            for (const std::string& msg : publicMessages) {
                sendMessage(client, QString::fromStdString(msg));
            }

            // приватные сообщения
            std::vector<Message> privateMessages = m_database->getUndeliveredPrivateMessages(id);
            for (const Message& msg : privateMessages) {
                QString sender = QString::fromStdString(msg.getSender());
                QString receiver = username;
                QString text = QString::fromStdString(msg.getText());
                sendMessage(client, "PMSG " + sender + " " + receiver + " " + text);
            }

            m_database->markMessagesAsDelivered(id);
        } else {
            sendMessage(client, "LOGIN_FAIL");
        }

    } else if (cmd == "MSG" && parts.size() >= 2) {
        if (m_clientIds.find(client) == m_clientIds.end()) {
            sendMessage(client, "ERROR Not logged in");
            return;
        }
        QString message = msg.section(' ', 1);
        QString sender = QString::fromStdString(m_database->getUserName(m_clientIds[client]));
        m_database->addChatMessage(sender.toStdString(), message.toStdString());
        broadcast("<" + sender + ">: " + message);

    } else if (cmd == "PMSG" && parts.size() >= 3) {
        if (m_clientIds.find(client) == m_clientIds.end()) {
            sendMessage(client, "ERROR Not logged in");
            return;
        }
        QString target = parts[1];
        QString message = msg.section(' ', 2);
        QString sender = QString::fromStdString(m_database->getUserName(m_clientIds[client]));

        if (m_database->addPrivateMessage(sender.toStdString(), target.toStdString(), message.toStdString())) {
            QString fullMsg = "PMSG " + sender + " " + target + " " + message;

            sendMessage(client, fullMsg); // ← Отправитель получит его и отобразит в приватном чате
            sendPrivateMessage(target, sender, message);
        } else {
            sendMessage(client, "PMSG_FAILED");
        }
    } else if (cmd == "USERLIST") {
            if (m_clientIds.find(client) == m_clientIds.end()) {
                sendMessage(client, "ERROR Not logged in");
                return;
            }
            QStringList users = m_database->getAllUsernames();
            QString response = "USERLIST " + users.join(" ");
            sendMessage(client, response);
        }
    else if (cmd == "HISTORY") {
        auto messages = m_database->getRecentMessages(50); // Получаем последние 50 сообщений
        for (const Message& msg : messages) {
            if (msg.getDest() == -1) { // Сообщение в общий чат
                QString text = QString("HISTORY_MSG %1 %2 %3\n")
                                   .arg(QString::fromStdString(msg.getSender()))
                                   .arg(msg.getTimestamp().toString(Qt::ISODate))
                                   .arg(QString::fromStdString(msg.getText()));

                client->write(text.toUtf8());
            } else {
                QString recipientName = QString::fromStdString(m_database->getUserName(msg.getDest()));
                QString text = QString("HISTORY_PMSG %1 %2 %3 %4\n")
                                   .arg(QString::fromStdString(msg.getSender()))
                                   .arg(recipientName)
                                   .arg(msg.getTimestamp().toString(Qt::ISODate))
                                   .arg(QString::fromStdString(msg.getText()));

                client->write(text.toUtf8());
            }
        }
    }
    else {
        sendMessage(client, "UNKNOWN_COMMAND");
    }
    // Добавить обработку других команд
}

QStringList Database::getAllUsernames() {
    QStringList users;
    QSqlQuery query("SELECT username FROM users");
    while (query.next()) {
        users << query.value(0).toString();
    }
    return users;
}

void Server::sendMessage(QTcpSocket* client, const QString& msg)
{
    if (!client) return;
    qDebug() << "[sendMessage] To client:" << client << "Message:" << msg;
    QByteArray data = msg.toUtf8();
    data.append('\n');
    client->write(data);
}

void Server::broadcast(const QString& msg)
{
    QMutexLocker locker(&m_clientsMutex);
    for (QTcpSocket* client : m_clients) {
        sendMessage(client, msg);
    }
}

void Server::sendPrivateMessage(const QString& targetUsername, const QString& sender, const QString& message)
{
     qDebug() << "[sendPrivateMessage] Looking for user:" << targetUsername;
    QMutexLocker locker(&m_clientsMutex);
    for (const auto& pair : m_clientIds) {
        QTcpSocket* sock = pair.first;
        int id = pair.second;
        QString uname = QString::fromStdString(m_database->getUserName(id));

        qDebug() << "[sendPrivateMessage] Checking:" << uname;

        if (uname == targetUsername) {
            QString fullMsg = "PMSG " + sender + " " + targetUsername + " " + message;
            qDebug() << "[sendPrivateMessage] Sending to" << uname << ":" << fullMsg;
            sendMessage(sock, fullMsg);
            break;
        }
    }
}

void Server::kickClient(QTcpSocket* client)
{
    if (!client) return;
    sendMessage(client, "You have been kicked by admin.");
    client->disconnectFromHost();
    QMutexLocker locker(&m_clientsMutex);
    m_clients.removeAll(client);
    m_clientIds.erase(client);
}

void Server::banClient(QTcpSocket* client)
{
    if (!client) return;
    QString ip = client->peerAddress().toString();
    m_bannedIps[ip] = true;
    sendMessage(client, "You have been banned.");
    client->disconnectFromHost();
    QMutexLocker locker(&m_clientsMutex);
    m_clients.removeAll(client);
    m_clientIds.erase(client);
}

bool Server::banUserByName(const std::string& username)
{
    bool success = m_database->setUserBanStatus(username, true);
    if (!success) return false;

    QMutexLocker locker(&m_clientsMutex);
    for (QTcpSocket* sock : m_clients) {
        if (m_clientIds.count(sock) && m_database->getUserName(m_clientIds[sock]) == username) {
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
    QMutexLocker locker(&m_clientsMutex);
    for (QTcpSocket* sock : m_clients) {
        if (m_clientIds.count(sock) &&
            m_database->getUserName(m_clientIds[sock]) == username) {
            kickClient(sock);
            return true;
        }
    }
    return false;
}
