// -*- mode: c++ -*-

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QFont>


class Application: public QApplication
{
    Q_OBJECT

public:
    Application (int&, char**);
    ~Application ();
};

#endif
