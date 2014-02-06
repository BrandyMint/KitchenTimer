#include "application.h"
#include "applicationmanager.h"
#include "resourcemanager.h"

#include <QFontDatabase>


Application::Application (int &argc, char **argv)
    : QApplication (argc, argv)
{
    setFont (QFont (QFontDatabase::applicationFontFamilies (QFontDatabase::addApplicationFont (":/fonts/Cartonsix NC.ttf")).at (0), 17));

    new ApplicationManager ();
    new ResourceManager ();
}
Application::~Application ()
{
    delete app_manager;
    delete resource_manager;
}
