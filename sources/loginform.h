#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <QWidget>
#include "client.h"

namespace Ui {
class LoginForm;
}

class LoginForm : public QWidget
{
    Q_OBJECT

public:
    explicit LoginForm(QWidget *parent = nullptr);
    ~LoginForm();
    void setClient(client* c);

signals:
    void registrationRequested();
    void accepted (int userId, QString userName, bool isAdmin);
    void rejected ();


private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_registrationpushButton_clicked();

    void onLoginResult(bool success, int userId, const QString& userName, bool isAdmin);

private:
    Ui::LoginForm *ui;
     client* m_client;
};

#endif // LOGINFORM_H
