// -*- mode: c++ -*-

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QStackedWidget>

class PageIntro;
class PageTimers;
class PageTimerEdit;
class PageDishSelect;

class MainWindow: public QStackedWidget
{
    Q_OBJECT

public:
    MainWindow ();
    ~MainWindow ();

public slots:
    void switchToPageIntro ();
    void switchToPageTimers ();
    void switchToPageTimerEdit ();
    void switchToPageDishSelect ();
    void setCurrentDish (int);
    void leavePageDishSelect ();
    void previousTimer ();
    void nextTimer ();
    void removeTimer (int);
    void cancelCurrentTimer ();
    void acceptCurrentTimer (const QString&, const QTime&);
    void editCurrentTimer ();
    void previousDish ();
    void nextDish ();
    void adjustTimerFromDishDetails (const QTime&, const QString&);

private:
    PageIntro *page_intro;
    PageTimers *page_timers;
    PageTimerEdit *page_timer_edit;
    PageDishSelect *page_dish_select;
};

#endif
