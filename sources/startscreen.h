#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include <QDialog>
#include <memory>
#include "client.h"
#include "adminpanel.h"

namespace Ui {
class StartScreen;
}

class StartScreen : public QDialog
{
    Q_OBJECT

public:
    explicit StartScreen(client* clientPtr,
                         QWidget *parent = nullptr);
    ~StartScreen();
    void setLoginForm(); //установить панель авторизации
    void setRegistrationForm(); // установить панель регистрации

    int userId() const;

    QString userName() const;

public slots:
    void onLoggedIn(uint userId, QString userName, bool isAdmin);
    void onRejectRequested();

signals:
    void accepted(uint userId, QString userName, bool isAdmin); //если вошел обычный пользователь
    void adminLoggedIn(); //сигнал для входа администратора

private:
    Ui::StartScreen *ui;
    int m_userId;
    QString m_userName;
     client* m_client;
};

#endif // STARTSCREEN_H
