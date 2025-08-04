#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include "client.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(int userId, QString userName,
                        client* clientPtr,
                        QWidget *parent = nullptr);
    ~MainWindow();
    static MainWindow* createClient();

    static int kInstanceCount;

private slots:
    void on_messageLineEdit_returnPressed();
    void on_SendMessageButton_clicked();
    void on_privateMessageSendButton_clicked();
    void on_actionOpen_another_client_triggered();
    void on_actionClose_this_client_triggered();

    void handlePublicMessage(const QString& from, const QString& text);
    void handlePrivateMessage(const QString& from, const QString& to, const QString& text);

    void on_themeToggleButton_clicked();

private:
    Ui::MainWindow *ui;
    client* m_client;
    int m_userId;
    QString m_userName;
};

#endif // MAINWINDOW_H
