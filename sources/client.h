#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QString>

class client: public QObject
{
    Q_OBJECT

public:
    explicit client(QObject* parent = nullptr);
    ~client();

    void connectToServer(const QString& host, quint16 port); // устанавливает соединение
    void disconnectFromServer();

    void login(const QString& username, const QString& password);
    void registerUser(const QString& username, const QString& password);
    void sendMessage(const QString& message);
    void sendPrivateMessage(const QString& recipient, const QString& message);
    void requestUserList();
    void requestHistory();

signals:
    void connected();
    void disconnected();
    void messageReceived(const QString& from, const QString& message);
    void privateMessageReceived(const QString& from, const QString& to, const QString& message);
    void userListReceived(const QStringList& users);
    void errorOccurred(const QString& error);
    void loginResult(bool success, int userId, const QString& username, bool isAdmin);
    void registerResult(bool success, int userId, const QString& username);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    void sendCommand(const QString& command);

    QTcpSocket* m_socket;
    int m_userId = -1;
    QString m_username;


};

#endif // CLIENT_H
