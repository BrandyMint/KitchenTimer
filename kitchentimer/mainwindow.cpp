#include "mainwindow.h"
#include "pageintro.h"
#include "pagetimer.h"
#include "pagetimerlist.h"
#include "pagedishselect.h"
#include "pagedishdetails.h"

MainWindow::MainWindow ()
    : QStackedWidget (), dish_predecessor_page (DishPredecessorPageUndefined)
{
    setWindowFlags (Qt::FramelessWindowHint);
    resize (540, 768);

    addWidget (page_intro = new PageIntro (this));
    addWidget (page_timer = new PageTimer (this));
    addWidget (page_timer_list = new PageTimerList (this));
    addWidget (page_dish_select = new PageDishSelect (this));
    addWidget (page_dish_details = new PageDishDetails (this));

    connect (page_intro, SIGNAL (switchToPageTimer ()), this, SLOT (switchToPageTimer ()));
    connect (page_timer, SIGNAL (switchToPageTimerList ()), this, SLOT (switchToPageTimerList ()));
    connect (page_timer, SIGNAL (switchToPageDishSelect ()), this, SLOT (switchToPageDishSelect ()));
    connect (page_timer_list, SIGNAL (switchToPageTimer ()), this, SLOT (switchToPageTimer ()));
    connect (page_timer_list, SIGNAL (switchToPageDishSelect ()), this, SLOT (switchToPageDishSelect ()));
    connect (page_dish_select, SIGNAL (switchToPageDishDetails ()), this, SLOT (switchToPageDishDetails ()));
    connect (page_dish_select, SIGNAL (leavePage ()), this, SLOT (leavePageDishDetails ()));
    connect (page_dish_details, SIGNAL (switchToPageDishSelect ()), this, SLOT (switchToPageDishSelect ()));
    connect (page_dish_details, SIGNAL (leavePage ()), this, SLOT (leavePageDishDetails ()));

    switchToPageIntro ();

    show ();
}
MainWindow::~MainWindow ()
{
}
void MainWindow::switchToPageIntro ()
{
    if (page_intro)
	setCurrentWidget (page_intro);
}
void MainWindow::switchToPageTimer ()
{
    if (page_timer)
	setCurrentWidget (page_timer);
}
void MainWindow::switchToPageTimerList ()
{
    if (page_timer_list)
	setCurrentWidget (page_timer_list);
}
void MainWindow::switchToPageDishSelect ()
{
    if (page_dish_select) {
	QWidget *page = currentWidget ();
	if (page == page_timer_list)
	    dish_predecessor_page = DishPredecessorPageTimerList;
	else if (page == page_timer)
	    dish_predecessor_page = DishPredecessorPageTimer;
	setCurrentWidget (page_dish_select);
    }
}
void MainWindow::switchToPageDishDetails ()
{
    if (page_dish_details)
	setCurrentWidget (page_dish_details);
}
void MainWindow::leavePageDishSelect ()
{
    if (dish_predecessor_page == DishPredecessorPageTimerList)
	setCurrentWidget (page_timer_list);
    else
	setCurrentWidget (page_timer);
}
void MainWindow::leavePageDishDetails ()
{
    if (dish_predecessor_page == DishPredecessorPageTimerList)
	setCurrentWidget (page_timer_list);
    else
	setCurrentWidget (page_timer);
}
