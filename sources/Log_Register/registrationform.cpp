#include "registrationform.h"
#include "ui_registrationform.h"
#include <QMessageBox>

registrationform::registrationform(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::registrationform),
      m_client(nullptr)
{
    ui->setupUi(this);
}

registrationform::~registrationform()
{
    delete ui;
}

void registrationform::setClient (client* c)
{
    m_client = c;
    connect(m_client, &client::registerResult, this, &registrationform::onRegisterResult);
}

void registrationform::on_loginButton_clicked()
{
 emit loginRequested();
}


void registrationform::on_buttonBox_accepted()
{
    if (!m_client) return;

    QString login = ui->LoginEdit->text().trimmed();
    QString password = ui->passwordEdit->text().trimmed();
    QString confirmPassword = ui->confirmpasswordEdit->text().trimmed();

    if (login.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Login cannot be empty"));
        return;
    }

    if (login.length() < 3) {
        QMessageBox::warning(this, tr("Error"), tr("Login must be at least 3 characters long"));
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::critical(this, tr("Error"), tr("Passwords do not match"));
        return;
    }

   m_client->registerUser(login, password);
}


void registrationform::on_buttonBox_rejected()
{
  emit rejected();
}

void registrationform::onRegisterResult(bool success, int userId, const QString& userName)
{
    if (success) {
        QMessageBox::information(this, tr("Registration Successful"), tr("You have been registered. Please login now."));
        this->close();
        emit loginRequested();
    } else {
        QMessageBox::critical(this, tr("Registration Failed"), tr("Username already exists or invalid data."));
    }
}
