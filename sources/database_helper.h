#ifndef DATABASE_HELPER_H
#define DATABASE_HELPER_H

#include <QSqlQuery>
#include <QString>

namespace database_helper
{
   void createTables(QSqlQuery &q); //создание таблиц сообщений и пользователей
   void printUsers(QSqlQuery &q); //печать пользователей в в Debug
   bool ensureAdminExists(QSqlQuery &q, const QString &username, const QString &passwordHash); //создание админа
}

#endif // DATABASE_HELPER_H
