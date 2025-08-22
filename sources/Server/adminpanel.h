#ifndef ADMINPANEL_H
#define ADMINPANEL_H

#include <QWidget>
#include <memory>
#include "Database/database.h"
#include "server.h"

namespace Ui
{
    class adminpanel;
}

class adminpanel : public QWidget
{
    Q_OBJECT

public:
    explicit adminpanel(std::shared_ptr<Database> dbPtr, Server* serverPtr, QWidget *parent = nullptr);
    ~adminpanel();

private slots:
    void on_refreshButton_clicked();
    void on_banUserButton_clicked();
    void on_unbanUserButton_clicked();
    void on_disconnectUserButton_3_clicked();
    void on_messageTypeComboBox_currentIndexChanged(int index);
    void on_userFilterComboBox_currentIndexChanged(int index);

private:
    void updateUserList();
    void updateMessageList();

    Ui::adminpanel *ui;
    std::shared_ptr<Database> m_dbPtr;
    Server* m_serverPtr;
};

#endif
