#include <QFontDatabase>
#include <QSettings>

#include "application.h"

Application::Application (int &argc, char **argv)
    : QApplication (argc, argv)
      // add_icon (":/images/add.png"),
{
    QFontDatabase::addApplicationFont (":/fonts/Cartonsix NC.ttf");

    base_font = QFont ("CartonsixNC", 17);

    setFont (base_font);

#if 0
    QSettings settings (KITCHENTIMER_SETTINGS_COMPANY_NAME, KITCHENTIMER_SETTINGS_PRODUCT_NAME);
    settings.beginGroup ("General");
    var_name = settings.value ("var_name", "default_value").toString ();
    settings.endGroup ();
#endif
}
Application::~Application ()
{
#if 0
    QSettings settings (KITCHENTIMER_SETTINGS_COMPANY_NAME, KITCHENTIMER_SETTINGS_PRODUCT_NAME);
    settings.beginGroup ("General");
    settings.setValue ("var_name", var_name);
    settings.endGroup ();
#endif
}
QFont &Application::getBaseFont ()
{
    return base_font;
}
