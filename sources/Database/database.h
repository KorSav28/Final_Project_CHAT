#pragma once
#include "Client/user.h"
#include "Client/message.h"

#include <vector>
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
    int searchUserByName(const string& username) const;

public:

	Database();

    vector<string> getUserList() const;
    string getUserName(int userId) const;

    vector<string> getChatMessages() const;
    vector<Message> getPrivateMessage(int userID = -1) const;
    vector<Message> getPrivateMessageForAdmin() const;

    bool connect();

    void debugPrintAllUsers() const;

    QStringList getAllUsernames();

    int addUser(const string& username, const string& password);
    int checkPassword(const string& username, const string& password);

    void addChatMessage(const string& sender, const string& message);
    bool addPrivateMessage(const string& sender, const string& target, const string& message);

    bool isUserBanned(const string& username) const;
    bool setUserBanStatus(const string& username, bool banned);
    vector<std::pair<string, bool>> getAllUsersWithBanStatus() const;

    std::vector<Message> getRecentMessages(int limit, int userId) const;

    bool isUserAdmin(const std::string& username) const;
};
