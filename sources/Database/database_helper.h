#ifndef DATABASE_HELPER_H
#define DATABASE_HELPER_H

#include <QSqlQuery>
#include <QString>

namespace database_helper
{
   void createTables(QSqlQuery &q);
   void printUsers(QSqlQuery &q);
   bool ensureAdminExists(QSqlQuery &q, const QString &username, const QString &passwordHash);
}

#endif
