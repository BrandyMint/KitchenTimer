#include "mainwindow.h"
#include "pageintro.h"
#include "pagetimers.h"
#include "pagetimeredit.h"
#include "pagedishselect.h"
#include "application.h"

#include <stdio.h>

MainWindow::MainWindow ()
    : QStackedWidget ()
{
    setWindowFlags (Qt::FramelessWindowHint);
    resize (540, 768);

    addWidget (page_intro = new PageIntro (this));
    addWidget (page_timers = new PageTimers (this));
    addWidget (page_timer_edit = new PageTimerEdit (this));
    addWidget (page_dish_select = new PageDishSelect (this));

    connect (page_intro, SIGNAL (switchToPageTimers ()), this, SLOT (switchToPageTimers ()));
    connect (page_timers, SIGNAL (switchToPageDishSelect ()), this, SLOT (switchToPageDishSelect ()));
    connect (page_timers, SIGNAL (previousTimer ()), this, SLOT (previousTimer ()));
    connect (page_timers, SIGNAL (nextTimer ()), this, SLOT (nextTimer ()));
    connect (page_timers, SIGNAL (removeTimer (int)), this, SLOT (removeTimer (int)));
    connect (page_timers, SIGNAL (editCurrentTimer ()), this, SLOT (editCurrentTimer ()));
    connect (page_timer_edit, SIGNAL (cancel ()), this, SLOT (cancelCurrentTimer ()));
    connect (page_timer_edit, SIGNAL (accept (const QString&, const QTime&)), this, SLOT (acceptCurrentTimer (const QString&, const QTime&)));
    connect (page_dish_select, SIGNAL (setCurrentDish (int)), this, SLOT (setCurrentDish (int)));
    connect (page_dish_select, SIGNAL (leavePage ()), this, SLOT (leavePageDishSelect ()));
    connect (page_dish_select, SIGNAL (previousDish ()), this, SLOT (previousDish ()));
    connect (page_dish_select, SIGNAL (nextDish ()), this, SLOT (nextDish ()));
    connect (page_dish_select, SIGNAL (adjustTimer (const QTime&, const QString&)),
	     this, SLOT (adjustTimerFromDishDetails (const QTime&, const QString&)));

    switchToPageIntro ();
}
MainWindow::~MainWindow ()
{
}
void MainWindow::switchToPageIntro ()
{
    setCurrentWidget (page_intro);
}
void MainWindow::switchToPageTimers ()
{
    page_timers->updateContent ();
    setCurrentWidget (page_timers);
}
void MainWindow::switchToPageTimerEdit ()
{
    page_timer_edit->updateContent ();
    setCurrentWidget (page_timer_edit);
}
void MainWindow::switchToPageDishSelect ()
{
    setCurrentWidget (page_dish_select);
}
void MainWindow::setCurrentDish (int plain_index)
{
    app->getReferenceModel ().setCurrentIndex (plain_index);
}
void MainWindow::leavePageDishSelect ()
{
    setCurrentWidget (page_timers);
}
void MainWindow::previousTimer ()
{
    int current_timer_index = app->getCurrentTimerIndex ();
    if (current_timer_index > 0) {
	app->setCurrentTimerIndex (current_timer_index - 1);
	page_timers->updateContentSubpageCurrentTimer ();
    }
}
void MainWindow::nextTimer ()
{
    QList<Timer*> &timers = app->getTimers ();
    int current_timer_index = app->getCurrentTimerIndex ();
    if (current_timer_index < (timers.count () - 1)) {
	app->setCurrentTimerIndex (current_timer_index + 1);
	page_timers->updateContentSubpageCurrentTimer ();
    }
}
void MainWindow::removeTimer (int index)
{
    QList<Timer*> &timers = app->getTimers ();
    if ((index >= 0) && (index < timers.count ())) {
	Timer *timer = timers.takeAt (index);
	delete timer;
	page_timers->updateContent ();
    }
}
void MainWindow::editCurrentTimer ()
{
    page_timer_edit->updateContent ();
    setCurrentWidget (page_timer_edit);
}
void MainWindow::cancelCurrentTimer ()
{
    page_timers->updateContent ();
    setCurrentWidget (page_timers);
}
void MainWindow::acceptCurrentTimer (const QString &title, const QTime &period)
{
    QList<Timer*> &timers = app->getTimers ();
    int current_timer_index = app->getCurrentTimerIndex ();
    if ((current_timer_index >= 0) && (current_timer_index < timers.count ())) {
	Timer *timer = timers.at (current_timer_index);
	timer->stop ();
	timer->setTitle (title);
	timer->setPeriod (period);
	timer->setTimeLeft (period);
    }
    page_timers->updateContent ();
    setCurrentWidget (page_timers);
}
void MainWindow::previousDish ()
{
    ReferenceModel &reference_model = app->getReferenceModel ();
    int current_index = reference_model.getCurrentIndex ();
    QList<ReferenceItem*> &plain_list = reference_model.getPlainList ();
    if (current_index > 0 && current_index < plain_list.size ()) {
	reference_model.setCurrentIndex (current_index - 1);
	page_dish_select->updateContentSubpageDetails ();
    }
}
void MainWindow::nextDish ()
{
    ReferenceModel &reference_model = app->getReferenceModel ();
    int current_index = reference_model.getCurrentIndex ();
    QList<ReferenceItem*> &plain_list = reference_model.getPlainList ();
    if (current_index >= 0 && current_index < (plain_list.size () - 1)) {
	reference_model.setCurrentIndex (current_index + 1);
	page_dish_select->updateContentSubpageDetails ();
    }
}
void MainWindow::adjustTimerFromDishDetails (const QTime &time, const QString &title)
{
    Timer *new_timer = new Timer (time, time, title);
    app->addTimer (new_timer);
    app->setCurrentTimerIndex (app->getTimers ().size () - 1);
    page_timers->updateContent ();
    setCurrentWidget (page_timers);
}
