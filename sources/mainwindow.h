#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include "client.h"

class Database;
class Server;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(int userId, QString userName,
                        client* clientPtr,std::shared_ptr<Database> db, Server* server,
                        QWidget *parent = nullptr);
    ~MainWindow();
    static MainWindow* createClient(std::shared_ptr<Database> db, Server* server);

    static int kInstanceCount;

private slots:
    void on_messageLineEdit_returnPressed();
    void on_SendMessageButton_clicked();
    void on_privateMessageSendButton_clicked();
    void on_actionOpen_another_client_triggered();
    void on_actionClose_this_client_triggered();

    void handlePublicMessage(const QString& from, const QString& text); //общий чат
    void handlePrivateMessage(const QString& from, const QString& to, const QString& text); //приватный чат

    void handleKickedByAdmin(); //отключен админом

    void on_themeToggleButton_clicked();//изменить стиль

private:
    Ui::MainWindow *ui;
    client* m_client;
    int m_userId;
    QString m_userName;
    std::shared_ptr<Database> m_db;
    Server* m_server;
};

#endif // MAINWINDOW_H
