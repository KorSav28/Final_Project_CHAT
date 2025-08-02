#include "loginform.h"
#include "ui_loginform.h"
#include <QMessageBox>

LoginForm::LoginForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginForm)
{
    ui->setupUi(this);
}

LoginForm::~LoginForm()
{
    delete ui;
}

void LoginForm::setDatabase(std::shared_ptr<Database> dbPtr)
{
    m_dbPtr = dbPtr;
}

void LoginForm::on_buttonBox_accepted()
{
     QString login = ui->loginEdit->text();//??????????????????????????????
    auto userId = m_dbPtr->checkPassword(ui->loginEdit->text().toStdString(),
                                             ui->passwordEdit->text().toStdString());
    if (userId == -1)
    {
        QMessageBox::critical(this, tr("Error"), tr("Password is wrong"));
        return;
    }

    // Проверка на бан
    if (m_dbPtr->isUserBanned(login.toStdString()))//??????????????????????
    {
        QMessageBox::warning(this, tr("Ban"), tr("User blocked by administrator"));
        return;
    }

 emit accepted(userId, ui->loginEdit->text());
}


void LoginForm::on_buttonBox_rejected()
{
 emit rejected();
}


void LoginForm::on_registrationpushButton_clicked()
{
 emit registrationRequested();
}

