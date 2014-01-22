#include <QApplication>
#include <QTranslator>

#include "application.h"
#include "mainwindow.h"

int main (int argc, char *argv[])
{
    Application application (argc, argv);
    QTranslator translator;
    translator.load (":/translations/ru");
    application.installTranslator (&translator);
    MainWindow window;
    return application.exec ();
}
