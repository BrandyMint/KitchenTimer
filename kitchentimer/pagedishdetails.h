// -*- mode: c++ -*-

#ifndef PAGEDISHDETAILS_H
#define PAGEDISHDETAILS_H

#include <QWidget>

class PageDishDetails: public QWidget
{
    Q_OBJECT

public:
    PageDishDetails (QWidget*);
    ~PageDishDetails ();

signals:
    void switchToPageDishSelect ();
    void leavePage ();
};

#endif
