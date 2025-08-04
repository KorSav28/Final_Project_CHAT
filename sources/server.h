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
#include "Database.h"

class Server : public QTcpServer
{
    Q_OBJECT

public:
    explicit Server(QObject* parent = nullptr);

    bool startServer(quint16 port); //запускает сервер на указанном порту
    void stopServer(); // отключает сервер
    void setDatabase(std::shared_ptr<Database> db);

    bool banUserByName(const std::string& username);
    bool unbanUserByName(const std::string& username);
    bool kickUserByName(const std::string& username);

private slots:
    void onNewConnection();
    void onReadyRead(); // клиент присылает данные
    void onClientDisconnect(); // клиент отключается

private:
    std::shared_ptr<Database> m_database; // база данных
    QVector<QTcpSocket*> m_clients; // список клиентов
    std::unordered_map<QTcpSocket*, int> m_clientIds;
    QHash<QString, bool> m_bannedIps;

    QMutex m_clientsMutex;

    void processCommand(QTcpSocket* client, const QString& msg);
    void sendMessage(QTcpSocket* client, const QString& msg);
    void broadcast(const QString& msg);
    void sendPrivateMessage(const QString& targetUsername, const QString& sender, const QString& message);

     void kickClient(QTcpSocket* client);
     void banClient(QTcpSocket* client);
};

#endif // SERVER_H
