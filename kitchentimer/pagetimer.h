// -*- mode: c++ -*-

#ifndef PAGETIMER_H
#define PAGETIMER_H

#include <QWidget>

class PageTimer: public QWidget
{
    Q_OBJECT

public:
    PageTimer (QWidget*);
    ~PageTimer ();

signals:
    void switchToPageTimerList ();
    void switchToPageDishSelect ();
};

#endif
