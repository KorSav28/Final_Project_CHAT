#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "startscreen.h"
#include <QDialog>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QTimer>
#include <QFile>
#include <QMessageBox>

int MainWindow::kInstanceCount = 0;

MainWindow::MainWindow(int userId, QString userName, client* clientPtr, QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_userId(userId),
    m_userName (userName),
    m_client(clientPtr)


{
    ui->setupUi(this);
    kInstanceCount++;

    setWindowTitle(QString("Chat - %1").arg(m_userName));

    connect(m_client, &client::messageReceived, this, &MainWindow::handlePublicMessage);
    connect(m_client, &client::privateMessageReceived, this, &MainWindow::handlePrivateMessage);

    m_client->requestUserList();

    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this]() {
        m_client->requestUserList();
    });
    timer->start(5000);
}

MainWindow::~MainWindow()
{
    delete ui;
    kInstanceCount--;
    if (kInstanceCount <= 0)
        qApp->exit(0);
}


MainWindow *MainWindow::createClient()
{
    client* cl = new client;
    cl->connectToServer("127.0.0.1", 12345); // ????

    StartScreen s (cl);
    s.setModal(true);
    QObject::connect(&s, &StartScreen::accepted, [&]() {
        qDebug() << "Login accepted";
    });

    s.exec();

    if (s.result() == QDialog::Accepted)
    {
        return new MainWindow(s.userId(), s.userName(), cl);
    }

    delete cl;
    return nullptr;
}

void MainWindow::on_messageLineEdit_returnPressed()
{
    on_SendMessageButton_clicked();
}


void MainWindow::on_SendMessageButton_clicked()
{
    QString text = ui->messageLineEdit->text();
    if (text.isEmpty())
        return;

    m_client->sendMessage(text);
    ui->messageLineEdit->clear();
}


void MainWindow::on_privateMessageSendButton_clicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Choose recipient");

    QVBoxLayout layout;
    QListWidget userList;
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    layout.addWidget(&userList);
    layout.addWidget(&buttons);
    dialog.setLayout(&layout);

    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Лямбда для обработки полученного списка пользователей
    auto onUserListReceived = [&](const QStringList& users) {
        userList.clear();
        for (const QString& name : users) {
            if (name != m_userName)  // исключаем себя из списка
                userList.addItem(name);
        }

        if (userList.count() == 0) {
            QMessageBox::information(this, tr("No users"), tr("No other users online."));
            dialog.reject();
            return;
        }

        if (dialog.exec() == QDialog::Accepted && userList.currentItem()) {
            QString recipient = userList.currentItem()->text();
            QString message = ui->messageLineEdit->text();
            if (!message.isEmpty()) {
                m_client->sendPrivateMessage(recipient, message);
                ui->messageLineEdit->clear();
            }
        }
    };

    // Объявляем переменную conn перед использованием в лямбде
    QMetaObject::Connection conn;

    // Подключаем сигнал, сохраняем соединение в conn
    conn = connect(m_client, &client::userListReceived,
                   this, [this, onUserListReceived, &conn](const QStringList& users) mutable {
                       onUserListReceived(users);
                       disconnect(conn); // отсоединяемся после первого вызова
                   });

    m_client->requestUserList();
}

void MainWindow::on_actionOpen_another_client_triggered()
{
   auto w = createClient();
    if(w)
    w->show();
}

void MainWindow::on_actionClose_this_client_triggered()
{
    this->close();
}

void MainWindow::handlePublicMessage(const QString& from, const QString& text)
{
    QString formatted = QString("<%1>: %2").arg(from, text);
    ui->commonChatBrowser->append(formatted);
}

void MainWindow::handlePrivateMessage(const QString& from, const QString& to, const QString& text)
{
    QString formatted = QString("<Private %1 → %2>: %3").arg(from, to, text);
    ui->privateChatBrowser->append(formatted);
}

void MainWindow::on_themeToggleButton_clicked()
{
    static bool darkMode = false;
    darkMode = !darkMode;

    QString stylePath = darkMode ? "styles/dark.qss" : "styles/light.qss";
    QFile styleFile(stylePath);
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString style = styleFile.readAll();
        qApp->setStyleSheet(style);
    }
}

