#include "loginform.h"
#include "ui_loginform.h"
#include <QMessageBox>

LoginForm::LoginForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginForm)
    , m_client(nullptr)
{
    ui->setupUi(this);
}

LoginForm::~LoginForm()
{
    delete ui;
}

void LoginForm::setClient(client* c)
{
    m_client = c;
    connect(m_client, &client::loginResult, this, &LoginForm::onLoginResult);
}

void LoginForm::on_buttonBox_accepted()
{
    if (!m_client) return;

    QString username = ui->loginEdit->text().trimmed();
    QString password = ui->passwordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Username and password cannot be empty"));
        return;
    }

   m_client->login(username, password);
}


void LoginForm::on_buttonBox_rejected()
{
 emit rejected();
}


void LoginForm::on_registrationpushButton_clicked()
{
 emit registrationRequested();
}

void LoginForm::onLoginResult(bool success, int userId, const QString& userName)
{
    if (success) {
        emit accepted(userId, userName);
    } else {
        QMessageBox::critical(this, tr("Login Failed"), tr("Invalid credentials or user is banned."));
    }
}
