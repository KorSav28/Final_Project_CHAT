#include "adminpanel.h"
#include "ui_adminpanel.h"
#include <QMessageBox>

adminpanel::adminpanel(std::shared_ptr<Database> dbPtr, Server* serverPtr,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::adminpanel)
    , m_dbPtr(dbPtr)
    , m_serverPtr(serverPtr)
{
    ui->setupUi(this);
    ui->messageTypeComboBox->addItems({"All", "Common", "Private"});
    updateUserList();
    updateMessageList();
}

adminpanel::~adminpanel()
{
    delete ui;
}

void adminpanel::updateUserList() // обновить список пользователей
{
    ui->userListWidget->clear();
    ui->userFilterComboBox->clear();
    ui->userFilterComboBox->addItem("All");

    auto users = m_dbPtr->getAllUsersWithBanStatus();
    for (const auto &pair : users)  //проход по всем парам (имя пользователя - статус бана)
    {
        QString userText = QString::fromStdString(pair.first);
        QListWidgetItem* item = new QListWidgetItem(userText);

        if (pair.second){              //если пользователь забанен, то пометка к имени + красный цвет
            userText += " [BANNED]";
            item->setText(userText);
            item->setForeground(Qt::red);
        }
        ui->userListWidget->addItem(item);
        ui->userFilterComboBox->addItem(QString::fromStdString(pair.first));
    }
}

void adminpanel::updateMessageList() // обновить список сообщений
{
    QString filterType = ui->messageTypeComboBox->currentText(); // получить выбранный тип сообщения
    QString filterUser = ui->userFilterComboBox->currentText(); // получить выбранного пользователя

    ui->messageBrowser->clear();

    auto commonMessages = m_dbPtr->getChatMessages();
    auto privateMessages = m_dbPtr->getPrivateMessageForAdmin();

    //обработка публичных сообщений
    if (filterType == "All" || filterType == "Common") {
        for (const auto &msg : commonMessages) {
            if (filterUser == "All" || QString::fromStdString(msg).contains(filterUser)) { //показываем сообщения, если "ALL" или имя выбранного пользователя
                ui->messageBrowser->append(QString::fromStdString(msg));
            }
        }
    }

    // обработка частных сообщений
    if (filterType == "All" || filterType == "Private") {
        for (const auto &msg : privateMessages) {
            QString sender = QString::fromStdString(msg.getSender());
            QString recipient = QString::fromStdString(m_dbPtr->getUserName(msg.getDest()));
            QString line = QString("<%1> to <%2>: %3")
                               .arg(sender)
                               .arg(recipient)
                               .arg(QString::fromStdString(msg.getText()));
            if (filterUser == "All" || line.contains(filterUser)) {
                ui->messageBrowser->append(line);
            }
        }
    }
}

void adminpanel::on_refreshButton_clicked() //обновить
{
    updateUserList();
    updateMessageList();
}


void adminpanel::on_banUserButton_clicked() //забанить
{
    auto item = ui->userListWidget->currentItem();
    if (!item) return;

    QString username = item->text().split(" ").first(); // получаем имя пользователя из строки

    if (m_dbPtr->setUserBanStatus(username.toStdString(), true)) {
        if (m_serverPtr)
            m_serverPtr->banUserByName(username.toStdString()); // сообщение серверу, что пользователь забанен

        QMessageBox::information(this, "Ban", username + " has been banned.");
        updateUserList();
    } else {
        QMessageBox::warning(this, "Error", "Failed to ban user.");
    }
}


void adminpanel::on_unbanUserButton_clicked() // разбанить
{
    auto item = ui->userListWidget->currentItem();
    if (!item) return;

   QString username = item->text().split(" ").first();

    if (m_dbPtr->setUserBanStatus(username.toStdString(), false)) {
        QMessageBox::information(this, "Unban", username + " has been unbanned.");
        updateUserList();
    } else {
        QMessageBox::warning(this, "Error", "Failed to unban user.");
    }
}


void adminpanel::on_disconnectUserButton_3_clicked() //отключить
{
    auto item = ui->userListWidget->currentItem();
    if (!item || !m_serverPtr) return;

    QString username = item->text().split(" ").first();

    if (m_serverPtr->kickUserByName(username.toStdString())) {
        QMessageBox::information(this, "Disconnect", username + " has been disconnected.");
        updateUserList();
    } else {
        QMessageBox::warning(this, "Error", "Failed to disconnect user.");
    }
}



void adminpanel::on_messageTypeComboBox_currentIndexChanged(int index)
{
    updateMessageList();
}


void adminpanel::on_userFilterComboBox_currentIndexChanged(int index)
{
    updateMessageList();
}

