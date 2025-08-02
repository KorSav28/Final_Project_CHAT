#pragma once
#include <vector>
#include "User.h"
#include "Message.h"
#include <string>
#include <unordered_map>

#include <QVariant>
#include <QSqlQuery>
#include <QSqlDatabase>

using std::string;
using std::vector;

class Database
{
    QSqlDatabase db;
    /*vector<User> _users;
	vector<Message> _messages;
    unordered_map<string, int> _usersMapByName;*/
    int searchUserByName(const string& username) const;

public:

	Database();

    vector<string> getUserList() const;
    string getUserName(int userId) const;
    vector<string> getChatMessages() const;//показать все сообщения
    vector<Message> getPrivateMessage(int userID = -1) const;//показать личные сообщения пользователю username

    bool connect();

    int addUser(const string& username, const string& password);
    int checkPassword(const string& username, const string& password);

    void addChatMessage(const string& sender, const string& message);
    bool addPrivateMessage(const string& sender, const string& target, const string& message);

    bool isUserBanned(const string& username) const;
    bool setUserBanStatus(const string& username, bool banned);
    vector<std::pair<string, bool>> getAllUsersWithBanStatus() const;
};
