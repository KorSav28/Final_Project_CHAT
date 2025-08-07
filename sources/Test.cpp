#include <iostream>
#include "CommandLineInterface.h"
#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include "server.h"
#include "Database.h"
#include "adminpanel.h"
using namespace std;

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);

   qDebug() << "Available SQL drivers:" << QSqlDatabase::drivers();

   // файлы для переводов
   QTranslator myappTranslator;
    myappTranslator.load("translations/my_ru.qm");
    a.installTranslator(&myappTranslator);

    QTranslator qtTranslator;
    qtTranslator.load("translations/qt_ru.qm");
    a.installTranslator(&qtTranslator);

    Server server;
    auto db = std::make_shared<Database>();
    server.setDatabase(db);

    if (!server.startServer(12345)) {
        qDebug() << "Не удалось запустить сервер:" << server.errorString();
        return -1;
    }

   auto w = MainWindow::createClient(db, &server);
   if (w)
       w->show();
   else
       return 0;

  return a.exec();
}
