// -*- mode: c++ -*-

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QStackedWidget>

class PageIntro;
class PageTimer;
class PageTimerList;
class PageDishSelect;
class PageDishDetails;

class MainWindow: public QStackedWidget
{
    Q_OBJECT

public:
    MainWindow ();
    ~MainWindow ();

public slots:
    void switchToPageIntro ();
    void switchToPageTimer ();
    void switchToPageTimerList ();
    void switchToPageDishSelect ();
    void switchToPageDishDetails ();
    void leavePageDishSelect ();
    void leavePageDishDetails ();

private:
    PageIntro *page_intro;
    PageTimer *page_timer;
    PageTimerList *page_timer_list;
    PageDishSelect *page_dish_select;
    PageDishDetails *page_dish_details;

    enum DishPredecessorPage {
	DishPredecessorPageUndefined,
	DishPredecessorPageTimer,
	DishPredecessorPageTimerList,
    } dish_predecessor_page;
};

#endif
