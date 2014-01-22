// -*- mode: c++ -*-

#ifndef PAGETIMERLIST_H
#define PAGETIMERLIST_H

#include <QWidget>

class PageTimerList: public QWidget
{
    Q_OBJECT

public:
    PageTimerList (QWidget*);
    ~PageTimerList ();

signals:
    void switchToPageTimer ();
    void switchToPageDishSelect ();
};

#endif
