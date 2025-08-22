#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include <QDialog>
#include <memory>
#include "Client/client.h"
#include "Server/adminpanel.h"

namespace Ui
{
    class StartScreen;
}

class StartScreen : public QDialog
{
    Q_OBJECT

public:
    explicit StartScreen(client* clientPtr,
                         QWidget *parent = nullptr);
    ~StartScreen();
    void setLoginForm();
    void setRegistrationForm();

    int userId() const;

    QString userName() const;

public slots:
    void onLoggedIn(uint userId, QString userName, bool isAdmin);
    void onRejectRequested();

signals:
    void accepted(uint userId, QString userName, bool isAdmin);
    void adminLoggedIn();

private:
    Ui::StartScreen *ui;
    int m_userId;
    QString m_userName;
     client* m_client;
};

#endif
