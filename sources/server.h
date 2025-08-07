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
    void setDatabase(std::shared_ptr<Database> db); //устанавливает БД

    bool banUserByName(const std::string& username); //бан
    bool unbanUserByName(const std::string& username); //разбан
    bool kickUserByName(const std::string& username); //отключение

private slots:
    void onNewConnection(); //клиент присоединился
    void onReadyRead(); // клиент присылает данные
    void onClientDisconnect(); // клиент отключается

private:
    std::shared_ptr<Database> m_database;
    QVector<QTcpSocket*> m_clients;
    std::unordered_map<QTcpSocket*, int> m_clientIds;
    QHash<QString, bool> m_bannedIps;

    QMutex m_clientsMutex;

    void processCommand(QTcpSocket* client, const QString& msg); //обработка команд
    void sendMessage(QTcpSocket* client, const QString& msg); //отправляет сообщение
    void broadcast(const QString& msg); //рассылает сообщение
    void sendPrivateMessage(const QString& targetUsername, const QString& sender, const QString& message); //отправляет првиатное сообщение

     void kickClient(QTcpSocket* client); //отключение
     void banClient(QTcpSocket* client); //бан
};

#endif // SERVER_H
