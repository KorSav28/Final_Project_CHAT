#include "Database.h"
#include "Parsing.h"
#include "sha1.h"
#include "database_helper.h"

#include <memory>
#include <iostream>
#include <QtSql>
#include <QDebug>

Database::Database()
{
    connect();
    if (connect()) {
        debugPrintAllUsers();
    } else {
        qDebug() << "Failed to connect to database";
    }
}

bool Database::connect()
{
    qDebug() << "Current working directory:" << QDir::currentPath();
    //настройка подключения к QSLite
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setHostName("localhost");
    db.setDatabaseName("chatdb");
    db.setUserName("chatuser");
    db.setPassword("yourpassword");

    if (!db.open())
    {
        qDebug() << "Database connection failed:" << db.lastError().text();
        return false;
    }

    QSqlQuery q; //для выполнения запросов

    database_helper::createTables(q);
    database_helper::printUsers(q);

    return database_helper::ensureAdminExists(q, "admin", QString::fromStdString(sha1_to_hex_string("admin123")));
}

int Database::addUser(const string& username, const string& password) //добавление нового пользователя
{
   QString uname = QString::fromStdString(username);
   QString passhash = QString::fromStdString(sha1_to_hex_string(password));
   qDebug() << "[addUser] Trying to add user:" << uname << "with hash:" << passhash;

    QSqlQuery q;
    q.prepare("INSERT INTO users (username, password) VALUES (:u, :p)");
    q.bindValue(":u", uname);
    q.bindValue(":p", passhash);

    if (q.exec()) {
        qDebug() << "[addUser] Insert successful. ID:" << q.lastInsertId();
        return q.lastInsertId().toInt();
    } else {
        qDebug() << "[addUser] Failed to insert user:" << q.lastError().text();
    }
    return -1;
}

int Database::checkPassword(const string& username, const string& password) //проверка логина и пароля
{
    QString uname = QString::fromStdString(username);
    QString passhash = QString::fromStdString(sha1_to_hex_string(password));
    qDebug() << "[checkPassword] Checking password for user:" << uname << ", hash:" << passhash;

    QSqlQuery q;

    // Проверка имени пользователя
    QSqlQuery qUserExists;
    qUserExists.prepare("SELECT COUNT(*) FROM users WHERE username = :u");
    qUserExists.bindValue(":u", uname);
    if (qUserExists.exec() && qUserExists.next()) {
        int count = qUserExists.value(0).toInt();
        qDebug() << "[checkPassword] User exists count for" << uname << ":" << count;
        if (count == 0) {
            qDebug() << "[checkPassword] User not found in database.";
            return -1;
        }
    } else {
        qDebug() << "[checkPassword] SQL error on user existence check:" << qUserExists.lastError().text();
        return -1;
    }

    // Проверка пароля
    q.prepare("SELECT id FROM users WHERE username = :u AND password = :p");
    q.bindValue(":u", uname);
    q.bindValue(":p", passhash);

    if (q.exec()) {
        if (q.next()) {
            int id = q.value(0).toInt();
            qDebug() << "[checkPassword] Login successful. User ID:" << id;
            return id;
        } else {
            qDebug() << "[checkPassword] Password does not match for user:" << uname;
        }
    } else {
        qDebug() << "[checkPassword] SQL error:" << q.lastError().text();
    }
    return -1;
}

int Database::searchUserByName(const string& username) const //поиск Id пользователя по имени
{
    QSqlQuery q;
    q.prepare("SELECT id FROM users WHERE username = :u");
    q.bindValue(":u", QString::fromStdString(username));

    if (q.exec() && q.next()) {
        return q.value(0).toInt();
    }
    return -1;
}

void Database::addChatMessage(const string& sender, const string& message) //добавить сообщения в БД для общего чата
{
    QSqlQuery q;
    q.prepare("INSERT INTO messages (sender, recipient, text) VALUES (:s, NULL, :m)");
    q.bindValue(":s", QString::fromStdString(sender));
    q.bindValue(":m", QString::fromStdString(message));
    q.exec();
}

bool Database::addPrivateMessage(const string& sender, const string& target, const string& message) //добавить сообщения в БД для приватного чата
{
    int recipientId = searchUserByName(target);
    if (recipientId == -1) return false;

    QSqlQuery q;
    q.prepare("INSERT INTO messages (sender, recipient, text) VALUES (:s, :r, :m)");
    q.bindValue(":s", QString::fromStdString(sender));
    q.bindValue(":r", recipientId);
    q.bindValue(":m", QString::fromStdString(message));
    return q.exec();
}

vector<string> Database::getUserList() const //получить список пользователей
{
    vector<string> users;
 QSqlQuery q("SELECT username FROM users");
    while (q.next()) {
        users.push_back(q.value(0).toString().toStdString());
    }
    return users;
}

string Database::getUserName(int userId) const //поиск имени по Id
{
    qDebug() << "DB: looking up username for id:" << userId;
    QSqlQuery q;
    q.prepare("SELECT username FROM users WHERE id = :id");
    q.bindValue(":id", userId);
    if (q.exec()&&q.next()){
        return q.value(0).toString().toStdString();
    }
    qDebug() << "DB: username NOT found for id:" << userId;
    return "";
}


vector<string> Database::getChatMessages() const //получить сообщения общего чата
{
    vector<string> result;
    QSqlQuery q ("SELECT sender, text FROM messages WHERE recipient IS NULL ORDER BY timestamp");
    while (q.next()) {
        std::string line = "<" + q.value(0).toString().toStdString() + ">: " + q.value(1).toString().toStdString();
        result.push_back(line);
    }
    return result;
}

std::vector<Message> Database::getPrivateMessage(int userID) const { //получить приватные сообщения
    std::vector<Message> messages;
    QSqlQuery q;

    q.prepare(R"(
        SELECT sender, recipient, text, timestamp
        FROM messages
        WHERE recipient IS NOT NULL AND
              (recipient = :id OR sender = (SELECT username FROM users WHERE id = :id))
        ORDER BY timestamp DESC
    )");

    q.bindValue(":id", userID);

    if (q.exec()) {
        while (q.next()) {
            std::string sender = q.value(0).toString().toStdString();
            int recipient = q.value(1).toInt();
            std::string text = q.value(2).toString().toStdString();
            QDateTime timestamp = q.value(3).toDateTime();
            int senderID = searchUserByName(sender);
            messages.emplace_back(senderID, sender, recipient, text, timestamp);
        }
    }

    // Разворачиваем в хронологическом порядке (от старых к новым)
    std::reverse(messages.begin(), messages.end());

    return messages;
}

std::vector<Message> Database::getPrivateMessageForAdmin() const { //получить приватные сообщения для админа
    std::vector<Message> messages;
    QSqlQuery q;

    q.prepare(R"(
        SELECT sender, recipient, text, timestamp
        FROM messages
        WHERE recipient IS NOT NULL
        ORDER BY timestamp DESC
    )");

    if (q.exec()) {
        while (q.next()) {
            std::string sender = q.value(0).toString().toStdString();
            int recipient = q.value(1).toInt();
            std::string text = q.value(2).toString().toStdString();
            QDateTime timestamp = q.value(3).toDateTime();
            int senderID = searchUserByName(sender);
            messages.emplace_back(senderID, sender, recipient, text, timestamp);
        }
    }

    std::reverse(messages.begin(), messages.end());
    return messages;
}

bool Database::isUserBanned(const string& username) const {    //проверка бан-статуса по имени
    QSqlQuery q;
    q.prepare("SELECT is_banned FROM users WHERE username = :u");
    q.bindValue(":u", QString::fromStdString(username));
    return q.exec() && q.next() && q.value(0).toBool();
}

bool Database::setUserBanStatus(const string& username, bool banned) { //установка бан-статуса по имени
    QSqlQuery q;
    q.prepare("UPDATE users SET is_banned = :b WHERE username = :u");
    q.bindValue(":b", banned);
    q.bindValue(":u", QString::fromStdString(username));
    return q.exec();
}

vector<std::pair<string, bool>> Database::getAllUsersWithBanStatus() const { //получить всех пользователей, у которых бан-статус
    vector<std::pair<string, bool>> result;
    QSqlQuery q("SELECT username, is_banned FROM users");
    while (q.next()) {
        string name = q.value(0).toString().toStdString();
        bool banned = q.value(1).toBool();
        result.emplace_back(name, banned);
    }
    return result;
}

void Database::debugPrintAllUsers() const //напечатать всех пользователей для debug
{
    qDebug() << "== DEBUG: All users and hashes ==";
    QSqlQuery q("SELECT id, username, password FROM users");
    while (q.next()) {
        qDebug() << "ID:" << q.value(0).toInt()
        << "Username:" << q.value(1).toString()
        << "Hash:" << q.value(2).toString();
    }
    qDebug() << "== END DEBUG ==";
}

std::vector<Message> Database::getRecentMessages(int limit, int userId) const {
    std::vector<Message> messages;
    QSqlQuery q;

    q.prepare(R"(
        SELECT sender, recipient, text, timestamp
        FROM messages
        WHERE
            recipient IS NULL OR
            recipient = :userId OR
            sender = (SELECT username FROM users WHERE id = :userId)
        ORDER BY timestamp DESC
        LIMIT :limit
    )");

    q.bindValue(":userId", userId);
    q.bindValue(":limit", limit);

    if (q.exec()) {
        while (q.next()) {
            std::string sender = q.value(0).toString().toStdString();
            int recipient = q.value(1).isNull() ? -1 : q.value(1).toInt();
            std::string text = q.value(2).toString().toStdString();
            QDateTime timestamp = q.value(3).toDateTime();
            int senderID = searchUserByName(sender);
            messages.emplace_back(senderID, sender, recipient, text, timestamp);
        }
    }

    std::reverse(messages.begin(), messages.end()); // старые сверху
    return messages;
}

bool Database::isUserAdmin(const std::string& username) const { //проверка статуса админа по имени
    QSqlQuery q;
    q.prepare("SELECT isadmin FROM users WHERE username = :u");
    q.bindValue(":u", QString::fromStdString(username));
    if (q.exec() && q.next()) {
        return q.value(0).toInt() == 1;
    }
    return false;
}
