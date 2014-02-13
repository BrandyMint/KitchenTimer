#include "mainwindow.h"
#include "pageintro.h"
#include "pagetimers.h"
#include "pagedishselect.h"
#include "pagesettings.h"
#include "applicationmanager.h"


MainWindow::MainWindow ()
    : QStackedWidget ()
{
    addWidget (page_intro = new PageIntro (this));
    addWidget (page_timers = new PageTimers (this));
    addWidget (page_dish_select = new PageDishSelect (this));
    addWidget (page_settings = new PageSettings (this));

    connect (page_intro, SIGNAL (switchToPageTimers ()), this, SLOT (switchToPageTimers ()));
    connect (page_timers, SIGNAL (switchToPageDishSelect ()), this, SLOT (switchToPageDishSelect ()));
    connect (page_timers, SIGNAL (stopCurrentTimer ()), this, SLOT (stopCurrentTimer ()));
    connect (page_timers, SIGNAL (setStartCurrentTimer (const QTime&)), this, SLOT (setStartCurrentTimer (const QTime&)));
    connect (page_timers, SIGNAL (analogTimerPressed ()), this, SLOT (soundAnalogTimerPressed ()));
    connect (page_timers, SIGNAL (analogTimerReleased ()), this, SLOT (soundAnalogTimerReleased ()));
    connect (page_timers, SIGNAL (analogTimerSlide ()), this, SLOT (soundAnalogTimerSlide ()));
    connect (page_timers, SIGNAL (zeroTimeReached ()), this, SLOT (signalManualAlarm ()));
    connect (page_timers, SIGNAL (longPressed ()), this, SLOT (signalLongPress ()));
    connect (page_timers, SIGNAL (showSettingsPageRequested ()), this, SLOT (switchToPageSettings ()));
    connect (page_dish_select, SIGNAL (setCurrentDish (int)), this, SLOT (setCurrentDish (int)));
    connect (page_dish_select, SIGNAL (leavePage ()), this, SLOT (leavePageDishSelect ()));
    connect (page_dish_select, SIGNAL (previousDish ()), this, SLOT (previousDish ()));
    connect (page_dish_select, SIGNAL (nextDish ()), this, SLOT (nextDish ()));
    connect (page_settings, SIGNAL (leavePage ()), this, SLOT (switchToPageTimers ()));

    setCurrentWidget (page_intro);
}
MainWindow::~MainWindow ()
{
}
void MainWindow::switchToPageTimers ()
{
    page_timers->updateContent ();
    setCurrentWidget (page_timers);
    if (page_intro) {
	delete page_intro;
	page_intro = NULL;
    }
}
void MainWindow::switchToPageDishSelect ()
{
    setCurrentWidget (page_dish_select);
}
void MainWindow::switchToPageSettings ()
{
    setCurrentWidget (page_settings);
}
void MainWindow::setCurrentDish (int plain_index)
{
    app_manager->reference_model.setCurrentIndex (plain_index);
}
void MainWindow::leavePageDishSelect ()
{
    setCurrentWidget (page_timers);
}
void MainWindow::stopCurrentTimer ()
{
    app_manager->getCurrentTimer ()->stop ();
}
void MainWindow::setStartCurrentTimer (const QTime &new_period)
{
    Timer *timer = app_manager->getCurrentTimer ();
    timer->stop ();
    timer->setTimeLeft (new_period);
    timer->start ();
}
void MainWindow::soundAnalogTimerPressed ()
{
    app_manager->alarm_sequencer.soundAnalogTimerPressed ();
}
void MainWindow::soundAnalogTimerReleased ()
{
    app_manager->alarm_sequencer.soundAnalogTimerReleased ();
}
void MainWindow::soundAnalogTimerSlide ()
{
    app_manager->alarm_sequencer.soundAnalogTimerSlide ();
}
void MainWindow::signalLongPress ()
{
    app_manager->alarm_sequencer.signalLongPress ();
}
void MainWindow::signalManualAlarm ()
{
    app_manager->alarm_sequencer.runManualAlarm ();
}
void MainWindow::cancelCurrentTimer ()
{
    page_timers->updateContent ();
    setCurrentWidget (page_timers);
}
void MainWindow::acceptCurrentTimer (const QString &title, const QTime &period)
{
    Timer *timer = app_manager->getCurrentTimer ();
    timer->stop ();
    timer->setTitle (title);
    timer->setTimeLeft (period);
}
void MainWindow::previousDish ()
{
    ReferenceModel &reference_model = app_manager->reference_model;
    int current_index = reference_model.getCurrentIndex ();
    QList<ReferenceItem*> &plain_list = reference_model.getPlainList ();
    if (current_index > 0 && current_index < plain_list.size ()) {
	reference_model.setCurrentIndex (current_index - 1);
	page_dish_select->updateContentSubpageDetails ();
    }
}
void MainWindow::nextDish ()
{
    ReferenceModel &reference_model = app_manager->reference_model;
    int current_index = reference_model.getCurrentIndex ();
    QList<ReferenceItem*> &plain_list = reference_model.getPlainList ();
    if (current_index >= 0 && current_index < (plain_list.size () - 1)) {
	reference_model.setCurrentIndex (current_index + 1);
	page_dish_select->updateContentSubpageDetails ();
    }
}
