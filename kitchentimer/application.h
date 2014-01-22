// -*- mode: c++ -*-

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QFont>
#include <QIcon>

#define app ((Application*) qApp)

namespace Registry {
    class RegionsTableModel;
    class DistrictsTableModel;
    class BuildingsTableModel;
    class AccountingCompaniesTableModel;
};

class Application: public QApplication
{
    Q_OBJECT

public:
    Application (int&, char**);
    ~Application ();
    QFont &getBaseFont ();

public:
    /* QIcon add_icon; */

private:
    QFont base_font;
};

#endif
