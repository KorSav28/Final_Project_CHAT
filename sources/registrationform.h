#ifndef REGISTRATIONFORM_H
#define REGISTRATIONFORM_H

#include <QWidget>
#include "client.h"

namespace Ui {
class registrationform;
}

class registrationform : public QWidget
{
    Q_OBJECT

public:
    explicit registrationform(QWidget *parent = nullptr);
    ~registrationform();
    void setClient(client* c);

signals:
    void loginRequested();
    void accepted (int userId, QString userName, bool isAdmin);
    void rejected ();

private slots:
    void on_loginButton_clicked();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

    void onRegisterResult(bool success, int userId, const QString& userName);

private:
    Ui::registrationform *ui;
    client* m_client;
};

#endif // REGISTRATIONFORM_H
