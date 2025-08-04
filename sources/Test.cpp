#include <iostream>
#include "CommandLineInterface.h"
#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include "server.h"
#include "Database.h"
using namespace std;

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);

   qDebug() << "Available SQL drivers:" << QSqlDatabase::drivers();

    QTranslator myappTranslator;
    myappTranslator.load("translations/my_ru.qm");
    a.installTranslator(&myappTranslator);

    QTranslator qtTranslator;
    qtTranslator.load("translations/qt_ru.qm");
    a.installTranslator(&qtTranslator);

    Server server;
    // если нужно, присоединяем базу данных
    auto db = std::make_shared<Database>();
    server.setDatabase(db);

    if (!server.startServer(12345)) {
        qDebug() << "Не удалось запустить сервер:" << server.errorString();
        return -1;
    }

   auto w = MainWindow::createClient();
   if (w)
       w->show();
   else
       return 0;

  return a.exec();
}
