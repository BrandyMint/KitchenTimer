#include "application.h"
#include "applicationmanager.h"
#include "resourcemanager.h"

#include <QFontDatabase>


Application::Application (int &argc, char **argv)
    : QApplication (argc, argv)
{
#ifdef Q_OS_MAC
    setFont (QFont ("Cartonsix NC", 20));
#else
    setFont (QFont (QFontDatabase::applicationFontFamilies (QFontDatabase::addApplicationFont (":/fonts/Cartonsix NC.ttf")).at (0), 30));
#endif

    new ApplicationManager ();
    new ResourceManager ();
}
Application::~Application ()
{
    delete app_manager;
    delete resource_manager;
}
