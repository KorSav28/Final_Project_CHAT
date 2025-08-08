#include "client.h"
#include <QDebug>
#include <QMessageBox>
#include <QApplication>

client::client(QObject* parent)
    : QObject(parent),
    m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected, this, &client::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &client::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &client::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &client::onErrorOccurred);

    init_command_processing_function();
}

client::~client()
{
    disconnectFromServer();
}

void client::connectToServer(const QString& host, quint16 port) //присоединение к серверу (инициирование подключения)
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

void client::disconnectFromServer() //отсоединение от сервера
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }
}

void client::sendCommand(const QString& command) //отправка команды серверу
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(command.toUtf8() + '\n');
    }else {
        qDebug() << "[client] Cannot send command, socket not connected.";
    }

}

void client::init_command_processing_function()
{
    command_processing.emplace("LOGIN_OK", [this](const QStringList &parts){
        if(parts.size() < 4)
            return false;
        bool ok;
        int userId = parts[1].toInt(&ok);
        QString username = parts[2];
        bool isAdmin = (parts[3] == "1");
        emit loginResult(true, userId, username, isAdmin);
        return true;
    });

    command_processing.emplace("LOGIN_FAIL", [this](const QStringList &parts){
        emit loginResult(false, -1, "", false);
        return true;
    });

    command_processing.emplace("REGISTER_OK", [this](const QStringList &parts){
        if(parts.size() < 3)
            return false;
        bool ok;
        int userId = parts[1].toInt(&ok);
        if (ok) emit registerResult(true, userId, parts[2]);
        return true;
    });

    command_processing.emplace("REGISTER_FAIL", [this](const QStringList &parts){
        emit registerResult(false, -1, "");
        return true;
    });

    command_processing.emplace("MSG", [this](const QStringList &parts){
        if(parts.size() < 3)
            return false;
        QString from = parts[1];
        QString text = parts.mid(2).join(" ");
        emit messageReceived(from, text);
        return true;
    });

    command_processing.emplace("PMSG", [this](const QStringList &parts){
        if(parts.size() < 4)
            return false;
        QString from = parts[1];
        QString to = parts[2];
        QString text = parts.mid(3).join(" "); // текст - все, что после второй позиции
        qDebug() << "[CLIENT] Got PMSG from" << from << "to" << to << ":" << text;
        emit privateMessageReceived(from, to, text);
        return true;
    });

    command_processing.emplace("USERLIST", [this](const QStringList &parts){
        if(parts.size() < 2)
            return false;
        QStringList users = parts.mid(1);
        emit userListReceived(users);
        return true;
    });

    command_processing.emplace("HISTORY_MSG", [this](const QStringList &parts){
        if(parts.size() < 4)
            return false;
        QString sender = parts[1];
        QString time = parts[2];
        QString text = parts.mid(3).join(" ");
        emit messageReceived(sender, "[" + time + "] " + text);
        return true;
    });

    command_processing.emplace("HISTORY_PMSG", [this](const QStringList &parts){
        if(parts.size() < 5)
            return false;
        QString sender = parts[1];
        QString receiver = parts[2];
        QString time = parts[3];
        QString text = parts.mid(4).join(" ");
        emit privateMessageReceived(sender, receiver, "[" + time + "] " + text);
        return true;
    });

    command_processing.emplace("SERVER", [this](const QStringList &parts){
        emit messageReceived("SERVER", parts.mid(1).join(" "));
        return true;
    });
}

void client::login(const QString& username, const QString& password) //авторизация
{
    qDebug() << "[client] login() called with username:" << username;
    sendCommand("LOGIN " + username + " " + password);
}

void client::registerUser(const QString& username, const QString& password) //регистрация
{
    sendCommand("REGISTER " + username + " " + password);
}

void client::sendMessage(const QString& message) //отправка общего сообщения
{
    sendCommand("MSG " + message);
}

void client::sendPrivateMessage(const QString& recipient, const QString& message) //отправка приватного сообщения
{
    sendCommand("PMSG " + recipient + " " + message);
}

void client::requestUserList() //получить список пользователей
{
    sendCommand("USERLIST");
}

void client::onConnected() //присоединение к серверу (обработка факта присоединения)
{
    qDebug() << "Connected to server.";
    emit connected();
}

void client::onDisconnected() //отсоединение
{
    qDebug() << "Disconnected from server.";
    emit disconnected();
}
void client::requestHistory() //запрос истории сообщений
{
    sendCommand("HISTORY");
}

void client::onReadyRead()
{
    while (m_socket->canReadLine()) {
        QString line = QString::fromUtf8(m_socket->readLine()).trimmed(); //читаем строку и удаляем пробелы в начале и конце
       qDebug()<< "Server says:" << line;

        if (line == "BAN") {
            QMessageBox::critical(nullptr, "Banned", "You have been banned by the admin.");
            QApplication::quit(); // Завершаем приложение
            return;
        }
        if (line == "KICK") {
            QMessageBox::warning(nullptr, "Disconnected", "You have been disconnected by the admin.");
            emit kickedbyAdmin(); //сообщаем дальше
            m_socket->disconnectFromHost(); //отключаем от сервера
            return;
        }

        QStringList parts = line.split(' '); //разбиваем строку для определения команды

        auto it = command_processing.find(parts[0].toStdString());
        if (it != command_processing.end()) {
            if (!it->second(parts)) {
                qDebug() << "Invalid arguments for command:" << parts[0];
            }
        } else {
            emit messageReceived("SERVER", line);
        }
    }
}

void client::onErrorOccurred(QAbstractSocket::SocketError socketError) //обработка ошибки сетевого сокета
{
    Q_UNUSED(socketError); //переменная не используется
    QString err = m_socket->errorString(); //текстовое сообщение об ошибки
    qDebug() << "Socket error:" << err;
    emit errorOccurred(err);
}
