#include "mainwindow.h"
#include "pageintro.h"
#include "pagetimers.h"
#include "pagesettings.h"
#include "applicationmanager.h"

#ifdef Q_OS_ANDROID
#  include <QAndroidJniEnvironment>
#  include <QAndroidJniObject>
#endif

MainWindow::MainWindow ()
{
    addWidget (page_intro = new PageIntro (this));
    addWidget (page_timers = new PageTimers (this));
    addWidget (page_settings = new PageSettings (this));

    connect (page_intro, SIGNAL (switchToPageTimers ()), this, SLOT (switchToPageTimers ()));
    connect (page_timers, SIGNAL (stopCurrentTimer ()), this, SLOT (stopCurrentTimer ()));
    connect (page_timers, SIGNAL (setStartCurrentTimer (const QTime&)), this, SLOT (setStartCurrentTimer (const QTime&)));
    connect (page_timers, SIGNAL (analogTimerPressed ()), this, SLOT (soundAnalogTimerPressed ()));
    connect (page_timers, SIGNAL (analogTimerReleased ()), this, SLOT (soundAnalogTimerReleased ()));
    connect (page_timers, SIGNAL (analogTimerSlide ()), this, SLOT (soundAnalogTimerSlide ()));
    connect (page_timers, SIGNAL (zeroTimeReached ()), this, SLOT (signalManualAlarm ()));
    connect (page_timers, SIGNAL (longPressed ()), this, SLOT (signalLongPress ()));
#ifdef KITCHENTIMER_DEBUG_BUILD
    connect (page_timers, SIGNAL (showSettingsPageRequested ()), this, SLOT (switchToPageSettings ()));
#endif
    connect (page_settings, SIGNAL (leavePage ()), this, SLOT (switchToPageTimers ()));

    connect (app_manager->getCurrentTimer (), SIGNAL (timeout ()), this, SLOT (showAbove ()));

#ifdef Q_OS_MAC
    switchToPageTimers ();
#else
    setCurrentWidget (page_intro);
#endif
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
void MainWindow::switchToPageSettings ()
{
    setCurrentWidget (page_settings);
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
    app_manager->alarm_sequencer->soundAnalogTimerPressed ();
}
void MainWindow::soundAnalogTimerReleased ()
{
    app_manager->alarm_sequencer->soundAnalogTimerReleased ();
}
void MainWindow::soundAnalogTimerSlide ()
{
    app_manager->alarm_sequencer->soundAnalogTimerSlide ();
}
void MainWindow::signalLongPress ()
{
    app_manager->alarm_sequencer->signalLongPress ();
}
void MainWindow::signalManualAlarm ()
{
    app_manager->alarm_sequencer->runManualAlarm ();
}
void MainWindow::showAbove ()
{
#ifdef Q_OS_ANDROID // TODO: Implement object method
    // JClass_Application = jni_env->FindClass ("com/brandymint/kitchentimer/MainActivity");
    // if (JClass_Application)
    // 	JMethod_wake = jni_env->GetMethodID (JClass_Application, "wake", "()V");
    QAndroidJniObject::callStaticMethod<void> ("com/brandymint/kitchentimer/MainActivity",
					       "wake",
					       "()V");
#endif
}
