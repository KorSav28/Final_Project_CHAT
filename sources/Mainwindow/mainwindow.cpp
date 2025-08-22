#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Log_Register/startscreen.h"
#include <QDialog>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QTimer>
#include <QFile>
#include <QMessageBox>

int MainWindow::kInstanceCount = 0;

MainWindow::MainWindow(int userId, QString userName, client* clientPtr, std::shared_ptr<Database> db, Server* server, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_userId(userId),
      m_userName (userName),
      m_server(server),
      m_db(db),
      m_client(clientPtr)


{
    ui->setupUi(this);
    kInstanceCount++;

    setWindowTitle(QString("Chat - %1").arg(m_userName));

    connect(m_client, &client::messageReceived, this, &MainWindow::handlePublicMessage);
    connect(m_client, &client::privateMessageReceived, this, &MainWindow::handlePrivateMessage);
    connect(m_client, &client::kickedbyAdmin, this, &MainWindow::handleKickedByAdmin);
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


MainWindow *MainWindow::createClient(std::shared_ptr<Database> db, Server* server)
{
    client* cl = new client;
    cl->connectToServer("127.0.0.1", 12345);

     StartScreen* s = new StartScreen(cl);
    s->setModal(true);

    MainWindow* mainWin = nullptr;

    QObject::connect(s, &StartScreen::accepted, [&](uint id, QString name, bool isAdmin) {
        if (!isAdmin) {
            qDebug() << "User logged in: " << name;
            mainWin = new MainWindow(id, name, cl, db, server);
            mainWin->show();
        }
    });

    QObject::connect(s, &StartScreen::adminLoggedIn, [=, &mainWin]() {
        qDebug() << "Admin logged in";

        mainWin = new MainWindow(1, "admin", cl, db, server);
        mainWin->setAttribute(Qt::WA_DeleteOnClose);
        mainWin->show();

        adminpanel* panel = new adminpanel(db, server);
        panel->setAttribute(Qt::WA_DeleteOnClose);
        panel->setWindowFlag(Qt::Window);
        panel->show();
    });


    s->exec();

    if (mainWin)
        return mainWin;

    delete cl;
    delete s;
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
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Choose recipient");

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    QListWidget* userList = new QListWidget(dialog);
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);

    layout->addWidget(userList);
    layout->addWidget(buttons);
    dialog->setLayout(layout);

    connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    auto onUserListReceived = [=](const QStringList& users) {
        userList->clear();
        for (const QString& name : users) {
            if (name != m_userName)
                userList->addItem(name);
        }

        if (userList->count() == 0) {
            QMessageBox::information(this, tr("No users"), tr("No other users online."));
            dialog->reject();
            dialog->deleteLater();
            return;
        }

        userList->setCurrentRow(0);

        if (dialog->exec() == QDialog::Accepted && userList->currentItem()) {
            QString recipient = userList->currentItem()->text();
            QString message = ui->messageLineEdit->text();
            if (!message.isEmpty()) {
                m_client->sendPrivateMessage(recipient, message);
                ui->messageLineEdit->clear();
            }
        }
        dialog->deleteLater();
    };

    QMetaObject::Connection* conn = new QMetaObject::Connection;
    *conn = connect(m_client, &client::userListReceived,
                    this, [=](const QStringList& users) {
                        QObject::disconnect(*conn);
                        delete conn;
                        onUserListReceived(users);
                    });
    m_client->requestUserList();
}

void MainWindow::on_actionOpen_another_client_triggered()
{
   auto w = createClient(m_db, m_server);
    if(w)
       w->show();
}

void MainWindow::on_actionClose_this_client_triggered()
{
    this->close();
}

void MainWindow::handlePublicMessage(const QString& from, const QString& text)
{
    qDebug() << "[UI] PUBLIC:" << from << text;
    QString formatted;
    if (from == "SERVER") {
        formatted = text;
    } else {
        formatted = QString("<%1>: %2").arg(from, text);
    }
    ui->commonChatBrowser->append(formatted);
}

void MainWindow::handlePrivateMessage(const QString& from, const QString& to, const QString& text)
{
    QString formatted;

    if (from == m_userName) {
        formatted = QString("<Вы → %1>: %2").arg(to, text);
    } else if (to == m_userName) {
        formatted = QString("<%1 → Вам>: %2").arg(from, text);
    } else {
        formatted = QString("<%1 → %2>: %3").arg(from, to, text);
    }

    ui->privateChatBrowser->append(formatted);
}

void MainWindow::on_themeToggleButton_clicked()
{
    qDebug() << "Theme";
    static bool darkMode = false;
    darkMode = !darkMode;

    QString basePath = QCoreApplication::applicationDirPath() + "/styles";
    QString stylePath = basePath + (darkMode ? "/dark.qss" : "/light.qss");
    qDebug() << "Loading style from:" << stylePath;
    QFile styleFile(stylePath);
    if (!styleFile.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Could not open style file:" << stylePath;
        return;
    }

    QString style = styleFile.readAll();
    qApp->setStyleSheet(style);
    qDebug() << "Style applied.";
}

void MainWindow::handleKickedByAdmin()
{
    this->close();

    m_client->connectToServer("127.0.0.1", 12345);

    StartScreen* start = new StartScreen(m_client);
    start->setModal(true);

    QObject::connect(start, &StartScreen::accepted, [&](uint id, QString name, bool isAdmin) {
        if (!isAdmin) {
            qDebug() << "User re-logged in: " << name;
            this->m_userId = id;
            this->m_userName = name;
            setWindowTitle(QString("Chat - %1").arg(m_userName));
            this->show();
        }
    });

    QObject::connect(start, &StartScreen::adminLoggedIn, [=]() {
        qDebug() << "Admin re-logged in";
        this->close();
        adminpanel* panel = new adminpanel(m_db, m_server);
        panel->setAttribute(Qt::WA_DeleteOnClose);
        panel->show();
    });

    start->exec();
}
