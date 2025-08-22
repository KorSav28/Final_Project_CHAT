#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>
#include <QVector>
#include <memory>
#include <unordered_map>
#include <string>
#include <QHash>
#include <QMutex>
#include "Database/database.h"

class Server : public QTcpServer
{
    Q_OBJECT

public:
    explicit Server(QObject* parent = nullptr);

    bool startServer(quint16 port);
    void stopServer();
    void setDatabase(std::shared_ptr<Database> db);

    bool banUserByName(const std::string& username);
    bool unbanUserByName(const std::string& username);
    bool kickUserByName(const std::string& username);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnect();

private:
    std::shared_ptr<Database> m_database;
    QVector<QTcpSocket*> m_clients;
    std::unordered_map<QTcpSocket*, int> m_clientIds;
    QHash<QString, bool> m_bannedIps;
    std::map<std::string, std::function<void(QTcpSocket*, const QStringList&)>> commandHandlers;

    QMutex m_clientsMutex;

    void initCommandHandlers();
    void processCommand(QTcpSocket* client, const QString& message);
    void sendMessage(QTcpSocket* client, const QString& message);
    void broadcast(const QString& message);
    void sendPrivateMessage(const QString& targetUsername, const QString& sender, const QString& message);

    void kickClient(QTcpSocket* client);
    void banClient(QTcpSocket* client);
};

#endif
