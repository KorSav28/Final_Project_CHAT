#include "database_helper.h"
#include <QDebug>
#include "sha1.h"
#include <QSqlError>

void database_helper::createTables(QSqlQuery &q)
{
    q.exec("CREATE TABLE IF NOT EXISTS users ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
           "username TEXT UNIQUE,"
           "password TEXT, "
           "is_banned BOOLEAN DEFAULT FALSE, "
           "isadmin INTEGER DEFAULT 0)");

    q.exec("CREATE TABLE IF NOT EXISTS messages ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
           "sender TEXT, "
           "recipient INTEGER, "
           "text TEXT, "
           "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
           "delivered BOOLEAN DEFAULT FALSE)");
}

void database_helper::printUsers(QSqlQuery &q)
{
    if (!q.exec("SELECT username FROM users")){
        qDebug() << "Users in database:"<< q.lastError().text();
        return;
    }
    while (q.next()) {
        qDebug() << q.value(0).toString();
    }
}

bool database_helper::ensureAdminExists(QSqlQuery &q, const QString &username, const QString &passwordHash)
{
    q.prepare("SELECT COUNT(*) FROM users WHERE username = :u");
    q.bindValue(":u", "admin");
    if (!q.exec()) {
        qDebug() << "Failed to check user existence:" <<q.lastError().text();
        return false;
    }
    q.next();

    if (q.value(0).toInt() == 0) {
        q.prepare("INSERT INTO users (username, password, isadmin) VALUES (:u, :p, 1)");
        q.bindValue(":u", "admin");
        q.bindValue(":p", QString::fromStdString(sha1_to_hex_string("admin123")));
        if (!q.exec()) {
            qDebug() << "Failed to insert test user:" << q.lastError().text();
            return false;
        } else {
            qDebug() << "Inserted test user 'admin'";
        }
    }
}
