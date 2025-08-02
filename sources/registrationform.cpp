#include "registrationform.h"
#include "ui_registrationform.h"
#include <QMessageBox>

registrationform::registrationform(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::registrationform)
{
    ui->setupUi(this);
}

registrationform::~registrationform()
{
    delete ui;
}

void registrationform::setDatabase(std::shared_ptr<Database> dbPtr)
{
    m_dbPtr = dbPtr;
}

void registrationform::on_loginButton_clicked()
{
 emit loginRequested();
}


void registrationform::on_buttonBox_accepted()
{
    QString login = ui->LoginEdit->text().trimmed();

    if (login.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Login cannot be empty"));
        return;
    }

    if (login.length() < 3) {
        QMessageBox::warning(this, tr("Error"), tr("Login must be at least 3 characters long"));
        return;
    }

    if (ui->passwordEdit->text() != ui->confirmpasswordEdit->text())
    {
        QMessageBox::critical(this, tr("Error"), tr("Passwords not match"));
        return;
    }
  auto userId = m_dbPtr->addUser(ui->LoginEdit->text().toStdString(),
                     ui->passwordEdit->text().toStdString());
    switch (userId)
  {
        case -1:
        QMessageBox::critical(this, tr("Error"), tr("Incorrect login"));
            return;
        case -2:
            QMessageBox::critical(this, tr("Error"), tr("Login already exists"));
            return;
        default:
            emit accepted(userId, ui->LoginEdit->text());
  }
}


void registrationform::on_buttonBox_rejected()
{
  emit rejected();
}

