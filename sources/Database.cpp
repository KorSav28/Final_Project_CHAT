#include "Database.h"
#include "Parsing.h"
#include "sha1.h"
#include <memory>
#include <iostream>
#include <QtSql>
#include <QDebug>

//'Ανθρωπος


Database::Database()
{
    connect();
}

bool Database::connect ()
{
    db = QSqlDatabase::addDatabase("QODBC");
    db.setHostName("localhost");
    db.setDatabaseName("chatdb");
    db.setUserName("chatuser");
    db.setPassword("yourpassword");

    if (!db.open()){
        qDebug() << "Database connection failed:" << db.lastError().text();
        return false;
    }

    QSqlQuery q;

    q.exec("ALTER TABLE users ADD COLUMN IF NOT EXISTS is_banned BOOLEAN DEFAULT FALSE");

    q.exec("CREATE TABLE IF NOT EXISTS users ("
           "id SERIAL PRIMARY KEY, "
           "username TEXT UNIQUE,"
           "password TEXT, "
           "is_banned BOOLEAN DEFAULT FALSE)");

    q.exec("CREATE TABLE IF NOT EXISTS messages ("
           "id SERIAL PRIMARY KEY, "
           "sender TEXT, "
           "recipient INTEGER, "
           "text TEXT, "
           "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP)");

    return true;
}

int Database::addUser(const string& username, const string& password)
{


   /* if (!correctName(username)) return -1;
    auto uit = _usersMapByName.find(username);
    if (uit != _usersMapByName.end()) return -2;
    User newUser = User(username, sha1(password));
    _users.push_back(newUser);
    _usersMapByName.insert({ username, newUser.getUserID() });
    return newUser.getUserID();*/
    QString uname = QString::fromStdString(username);
   //УТОЧНИТЬ ПО ФУНКЦИИ SHA1
   QString passhash = QString::fromStdString(sha1_to_hex_string(password));

    QSqlQuery q;
   q.prepare("INSERT INTO users (username, password) VALUES (:u, :p) RETURNING id");
   q.bindValue(":u", uname);
   q.bindValue(":p", passhash);

   if (q.exec() && q.next()) {
       return q.value(0).toInt();
   }
   return -1;
}

int Database::checkPassword(const string& username, const string& password)
{
   /* int result = -1;
    Hash passHash = sha1(password);
    for (const auto &u : _users)
    {
        result = u.checklogin(username, passHash);
        if (result != -1) return result;
    }
    return result; */
    QString uname = QString::fromStdString(username);
    //УТОЧНИТЬ ПО ФУНКЦИИ SHA1
    QString passhash = QString::fromStdString(sha1_to_hex_string(password));

    QSqlQuery q;
    q.prepare("SELECT id FROM users WHERE username = :u AND password = :p");
    q.bindValue(":u", uname);
    q.bindValue(":p", passhash);

    if (q.exec() && q.next()) {
        return q.value(0).toInt();
    }
    return -1;
}

int Database::searchUserByName(const string& username) const
{
    /*auto uit = _usersMapByName.find(username);
	if (uit != _usersMapByName.end()) return uit->second;
  return -1;*/

    QSqlQuery q;
    q.prepare("SELECT id FROM users WHERE username = :u");
    q.bindValue(":u", QString::fromStdString(username));

    if (q.exec() && q.next()) {
        return q.value(0).toInt();
    }
    return -1;
}

void Database::addChatMessage(const string& sender, const string& message)
{
    /*_messages.push_back(Message(sender, text));*/
    QSqlQuery q;
    q.prepare("INSERT INTO messages (sender, recipient, text) VALUES (:s, NULL, :m)");
    q.bindValue(":s", QString::fromStdString(sender));
    q.bindValue(":m", QString::fromStdString(message));
    q.exec();
}

bool Database::addPrivateMessage(const string& sender, const string& target, const string& message)
{
    /*int targetUser = searchUserByName(target);
    if (targetUser < 0)
    {
        return false;
    }
    _messages.push_back(Message(sender, targetUser, message));
    return true;*/

    int recipientId = searchUserByName(target);
    if (recipientId == -1) return false;

    QSqlQuery q;
    q.prepare("INSERT INTO messages (sender, recipient, text) VALUES (:s, :r, :m)");
    q.bindValue(":s", QString::fromStdString(sender));
    q.bindValue(":r", recipientId);
    q.bindValue(":m", QString::fromStdString(message));
    return q.exec();
}

vector<string> Database::getUserList() const
{
 /* vector<string> userList;
  for(auto user : _usersMapByName)
  {
    userList.push_back(user.first);
  }
  return userList;*/
    vector<string> users;
 QSqlQuery q("SELECT username FROM users");
    while (q.next()) {
        users.push_back(q.value(0).toString().toStdString());
    }
    return users;
}

string Database::getUserName(int userId) const
{
  /*for (auto it = _usersMapByName.begin(); it != _usersMapByName.end(); ++it) {
    if (it->second == userId)
      return it->first;
  }
  return "";*/
    QSqlQuery q;
    q.prepare("SELECT username FROM users WHERE id = :id");
    q.bindValue(":id", userId);
    if (q.exec()&&q.next()){
        return q.value(0).toString().toStdString();
    }
    return "";
}


vector<string> Database::getChatMessages() const
{
    /*vector<string> strings;
	for (auto &m: _messages)
	{
		if (m.getDest() == -1)
		{
			strings.push_back("<" + m.getSender() + ">: " + m.getText());
		}
	}
    return strings;*/

    vector<string> result;
    QSqlQuery q ("SELECT sender, text FROM messages WHERE recipient IS NULL ORDER BY timestamp");
    while (q.next()) {
        string line = "<" + q.value(0).toString().toStdString() + ">: " + q.value(1).toString().toStdString();
        result.push_back(line);
    }
    return result;
}

vector<Message> Database::getPrivateMessage(int userID) const
{
    /*vector<Message> strings;
	//int userID = searchUserByName(username);
	for (auto &m : _messages)
  {
    if(userID == -1 && m.getDest() != -1)
      strings.push_back(m);
    else if(userID != -1 && m.getDest() == userID)
      strings.push_back(m);
	}
    return strings;*/
    vector<Message> messages;
    QSqlQuery q;
    if (userID == -1) {
        q.prepare("SELECT sender, recipient, text FROM messages WHERE recipient IS NOT NULL ORDER BY timestamp");
    } else {
        q.prepare("SELECT sender, recipient, text FROM messages WHERE recipient = :id ORDER BY timestamp");
        q.bindValue(":id", userID);
    }

    if (q.exec()) {
        while (q.next()) {
            string sender = q.value(0).toString().toStdString();
            int recipient = q.value(1).toInt();
            string text = q.value(2).toString().toStdString();
            messages.emplace_back(sender, recipient, text);
        }
    }
    return messages;
}

bool Database::isUserBanned(const string& username) const {
    QSqlQuery q;
    q.prepare("SELECT is_banned FROM users WHERE username = :u");
    q.bindValue(":u", QString::fromStdString(username));
    return q.exec() && q.next() && q.value(0).toBool();
}

bool Database::setUserBanStatus(const string& username, bool banned) {
    QSqlQuery q;
    q.prepare("UPDATE users SET is_banned = :b WHERE username = :u");
    q.bindValue(":b", banned);
    q.bindValue(":u", QString::fromStdString(username));
    return q.exec();
}

vector<std::pair<string, bool>> Database::getAllUsersWithBanStatus() const {
    vector<std::pair<string, bool>> result;
    QSqlQuery q("SELECT username, is_banned FROM users");
    while (q.next()) {
        string name = q.value(0).toString().toStdString();
        bool banned = q.value(1).toBool();
        result.emplace_back(name, banned);
    }
    return result;
}
