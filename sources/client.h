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
    void disconnectFromServer(); //разрывает соединение

    void login(const QString& username, const QString& password); //логин
    void registerUser(const QString& username, const QString& password); //регистрация

    void sendMessage(const QString& message); // отправить сообщение всем
    void sendPrivateMessage(const QString& recipient, const QString& message); // отправить сообщение приватно

    void requestUserList(); // предоставить список пользователей
    void requestHistory(); // предоставить историю сообщений

signals:
    void connected(); // подключился к серверу
    void disconnected(); //отключился от сервера

    void messageReceived(const QString& from, const QString& message); // получено общее сообщение
    void privateMessageReceived(const QString& from, const QString& to, const QString& message); // получено приватное сообщение
    void userListReceived(const QStringList& users); // получен список пользователей

    void loginResult(bool success, int userId, const QString& username, bool isAdmin); // реазультат авторизации
    void registerResult(bool success, int userId, const QString& username); // результат регистрации

    void errorOccurred(const QString& error); //произошла ошибка
    void kickedbyAdmin(); //пользователь был отключен администратором

private slots:
    void onConnected(); //соединение установлено
    void onDisconnected(); //соединение разорвано
    void onReadyRead(); // данные готовы к чтению
    void onErrorOccurred(QAbstractSocket::SocketError socketError); // ошибка сокета

private:
    void sendCommand(const QString& command); // отправить строковую команду на сервер

    QTcpSocket* m_socket;
    int m_userId = -1;
    QString m_username;
};

#endif // CLIENT_H
