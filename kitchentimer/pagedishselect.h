// -*- mode: c++ -*-

#ifndef PAGEDISHSELECT_H
#define PAGEDISHSELECT_H

#include <QWidget>

class PageDishSelect: public QWidget
{
    Q_OBJECT

public:
    PageDishSelect (QWidget*);
    ~PageDishSelect ();

signals:
    void leavePage ();
    void switchToPageDishDetails ();
};

#endif
