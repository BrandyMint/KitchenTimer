#include "application.h"
#include "mainwindow.h"

#include <QTranslator>


int main (int argc, char *argv[])
{
    Application application (argc, argv);
    QTranslator translator;
    translator.load (":/translations/ru");
    application.installTranslator (&translator);
    MainWindow window;
    // TODO: Hide following for non-desktop platforms
    window.setWindowFlags (Qt::FramelessWindowHint);
    window.resize (540, 768);
#ifdef Q_WS_S60
    window.showMaximized ();
#else
    window.show ();
#endif
#ifdef Q_OS_MAC
    window.raise ();
#endif
    return application.exec ();
}
