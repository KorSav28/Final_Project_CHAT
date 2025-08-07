#ifndef ADMINPANEL_H
#define ADMINPANEL_H

#include <QWidget>
#include <memory>
#include "Database.h"
#include "server.h"

namespace Ui {
class adminpanel;
}

class adminpanel : public QWidget
{
    Q_OBJECT

public:
    explicit adminpanel(std::shared_ptr<Database> dbPtr, Server* serverPtr, QWidget *parent = nullptr);
    ~adminpanel();

private slots:
    void on_refreshButton_clicked(); // обновить все
    void on_banUserButton_clicked(); // забанить пользователя
    void on_unbanUserButton_clicked(); // разбанить пользователя
    void on_disconnectUserButton_3_clicked(); // отключить пользователя
    void on_messageTypeComboBox_currentIndexChanged(int index); // выбор типа сообщения (все, общие, приватные)
    void on_userFilterComboBox_currentIndexChanged(int index); // выбор пользователя для просмотра сообщений

private:
    void updateUserList(); // обновить список пользователей
    void updateMessageList(); // обновить список сообщений

    Ui::adminpanel *ui;
    std::shared_ptr<Database> m_dbPtr;
    Server* m_serverPtr;
};

#endif // ADMINPANEL_H
