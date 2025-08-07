#pragma once
#include "User.h"
#include "Message.h"

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
    int searchUserByName(const string& username) const; //поиск пользователя по имени

public:

	Database();

    vector<string> getUserList() const; //получить всех пользователей
    string getUserName(int userId) const; // получить имя пользователя по ID

    vector<string> getChatMessages() const;//показать все сообщения
    vector<Message> getPrivateMessage(int userID = -1) const; //показать приватные сообщения
    vector<Message> getPrivateMessageForAdmin() const; //показать приватные сообщения для админа

   bool connect(); //подключение к БД

    void debugPrintAllUsers() const;//для вывода пользователей в консоль

    QStringList getAllUsernames(); //получить всех пользователей в формате QString

    int addUser(const string& username, const string& password); //добавить нового пользователя
    int checkPassword(const string& username, const string& password); //проверка пароля

    void addChatMessage(const string& sender, const string& message); //добавить общие сообщения
    bool addPrivateMessage(const string& sender, const string& target, const string& message); //добавить приватные сообщения

    //проверка и установка статуса бана
    bool isUserBanned(const string& username) const;
    bool setUserBanStatus(const string& username, bool banned);
    vector<std::pair<string, bool>> getAllUsersWithBanStatus() const; // получить список пользователей со статусом бан

    /*std::vector<Message> getUndeliveredPrivateMessages(int userId) const; //получить недоставленные сообщения
    bool markMessagesAsDelivered(int userId); // пометить все сообщения как доставленные*/

    std::vector<Message> getRecentMessages(int limit, int userId) const; //получить последние 50 сообщений

    bool isUserAdmin(const std::string& username) const; //проверка статуса админа
};
