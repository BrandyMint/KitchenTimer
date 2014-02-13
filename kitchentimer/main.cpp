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

#if defined(Q_OS_ANDROID)
#elif defined(Q_OS_MAC)
#else
    window.setWindowFlags (Qt::FramelessWindowHint);
    window.resize (540, 768);
#endif

    window.show ();

#ifdef Q_OS_MAC
    window.raise ();
#endif

    return application.exec ();
}
