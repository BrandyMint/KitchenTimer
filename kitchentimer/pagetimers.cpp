#include "pagetimers.h"
#include "applicationmanager.h"
#include "widgets/analogtimer.h"
#include "widgets/digitaltimer.h"
#include "widgets/clickablelabel.h"
#include "widgets/buttonstick.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScroller>
#include <QScrollArea>
#include <QStackedWidget>
#include <QDial>
#include <QMessageBox>


PageTimers::PageTimers (QWidget *parent)
    : Background (parent), edit_mode (false)
{
    QGridLayout *layout = new QGridLayout (this);
    layout->setMargin (0);
    layout->setSpacing (0);

    {
	digital_timer = new DigitalTimer (this);
	connect (digital_timer, SIGNAL (enterEditModeRequested ()), this, SLOT (enterEditMode ()));
	connect (digital_timer, SIGNAL (timeChanged (const QTime&)), this, SLOT (digitalTimeChanged (const QTime&)));
	layout->addWidget (digital_timer, 1, 1);
    }

    {
	analog_timer = new AnalogTimer (this);
	connect (analog_timer, SIGNAL (timeChanged (const QTime&)), this, SLOT (analogTimeChanged (const QTime&)));
	connect (analog_timer, SIGNAL (clearAlarms ()), this, SLOT (clearCurrentAlarms ()));
	connect (analog_timer, SIGNAL (enterEditModeRequested ()), this, SLOT (enterEditModePressed ()));
	connect (analog_timer, SIGNAL (leaveEditModeRequested ()), this, SLOT (leaveEditMode ()));
	connect (analog_timer, SIGNAL (pressed ()), this, SIGNAL (analogTimerPressed ()));
	connect (analog_timer, SIGNAL (released ()), this, SIGNAL (analogTimerReleased ()));
	layout->addWidget (analog_timer, 2, 1);
    }

    layout->setColumnStretch (0, 15);
    layout->setColumnStretch (1, 50);
    layout->setColumnStretch (2, 15);

    layout->setRowStretch (0, 2);
    layout->setRowStretch (1, 40);
    layout->setRowStretch (2, 40);
    layout->setRowStretch (3, 10);
    layout->setRowStretch (4, 2);
    
    button_stick = new ButtonStick (this);
    button_stick->setAudioEnabled (app_manager->getAudioEnabled ());
    connect (button_stick, SIGNAL (released ()), app_manager, SLOT (toggleAudioEnabled ()));
    connect (app_manager, SIGNAL (valueChangedAudioEnabled (bool)), button_stick, SLOT (setAudioEnabled (bool)));
    connect (button_stick, SIGNAL (longPressed ()), app_manager, SLOT (signalLongPress ()));
    connect (button_stick, SIGNAL (longPressed ()), this, SIGNAL (showSettingsPageRequested ()));

#if 0 // Link with Reference
    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	hlayout->addStretch (1);
	QPushButton *add_timer_button = new QPushButton ("Add timer", this);
	connect (add_timer_button, SIGNAL (clicked ()), this, SIGNAL (switchToPageDishSelect ()));
	hlayout->addWidget (add_timer_button);
	hlayout->addStretch (1);
	layout->addLayout (hlayout);
	add_timer_button->hide ();
    }
#endif
    
    connect (this, SIGNAL (pressed ()), this, SLOT (leaveEditMode ()));

    connect (app_manager->getCurrentTimer (), SIGNAL (timeout ()), this, SLOT (timeout ()));

    updateContent ();

    connect (&update_timer, SIGNAL (timeout ()), this, SLOT (updateContent ()));
    update_timer.setInterval (100);
    update_timer.setSingleShot (false);
    update_timer.start ();

    leave_edit_mode_timer.setSingleShot (false);
    connect (&leave_edit_mode_timer, SIGNAL (timeout ()), this, SLOT (leaveEditMode ()));

    connect (analog_timer, SIGNAL (lmb_pressed ()), &leave_edit_mode_timer, SLOT (stop ()));
    connect (digital_timer, SIGNAL (lmb_pressed ()), &leave_edit_mode_timer, SLOT (stop ()));
    connect (analog_timer, SIGNAL (userIsAlive ()), this, SLOT (restartTirednessTimer ()));
    connect (digital_timer, SIGNAL (userIsAlive ()), this, SLOT (restartTirednessTimer ()));
    connect (analog_timer, SIGNAL (lmb_released ()), this, SLOT (startTirednessTimer ()));
    connect (digital_timer, SIGNAL (lmb_released ()), this, SLOT (startTirednessTimer ()));
}
PageTimers::~PageTimers ()
{
}
void PageTimers::updateContent ()
{
    if (!analog_timer->isSliderDown ()) {
	Timer *current_timer = app_manager->getCurrentTimer ();
	QTime tm = current_timer->getTimeLeft ();
	digital_timer->setTime (current_timer->getTimeLeft ());
	analog_timer->setTime (current_timer->getTimeLeft ());
    }
}
void PageTimers::resizeEvent (QResizeEvent*)
{
    button_stick->adjustGeometry (rect ());
}
void PageTimers::dialValueChanged (int new_period_sec)
{
    dial_value = QTime (0, 0, 0).addSecs (new_period_sec);
    digital_timer->setTime (dial_value);
}
void PageTimers::analogTimeChanged (const QTime &new_time)
{
    dial_value = new_time;
    digital_timer->setTime (dial_value);
    currentTimerAdjusted ();
}
void PageTimers::digitalTimeChanged (const QTime &new_time)
{
    dial_value = new_time;
    analog_timer->setTime (dial_value);
    currentTimerAdjusted ();
}
void PageTimers::currentTimerAdjusted ()
{
    Timer *current_timer = app_manager->getCurrentTimer ();
    current_timer->setTimeLeft (dial_value);
}
void PageTimers::clearCurrentAlarms ()
{
    app_manager->clearAlarms ();
}
void PageTimers::enterEditMode ()
{
    stopCurrentTimer ();
    if (!edit_mode) {
	edit_mode = true;
	app_manager->clearAlarms ();
	if (app_manager->edition_happened) {
	    analog_timer->enterEditMode (KITCHENTIMER_EDIT_TRANSITION_TIMEOUT_MS);
	    digital_timer->enterEditMode (KITCHENTIMER_EDIT_TRANSITION_TIMEOUT_MS);
	    startShading (KITCHENTIMER_EDIT_TRANSITION_TIMEOUT_MS);
	} else {
	    analog_timer->enterEditMode (KITCHENTIMER_INITIAL_EDIT_TRANSITION_TIMEOUT_MS);
	    digital_timer->enterEditMode (KITCHENTIMER_INITIAL_EDIT_TRANSITION_TIMEOUT_MS);
	    startShading (KITCHENTIMER_INITIAL_EDIT_TRANSITION_TIMEOUT_MS);
	    app_manager->edition_happened = true;
	}
	app_manager->getCurrentTimer ()->stop ();
	// leave_edit_mode_timer.start (KITCHENTIMER_LEAVE_EDIT_MODE_TIMEOUT_MS);
    }
}
void PageTimers::enterEditModePressed ()
{
    stopCurrentTimer ();
    if (!edit_mode) {
	edit_mode = true;
	app_manager->clearAlarms ();
	if (app_manager->edition_happened) {
	    analog_timer->enterEditModePressed (KITCHENTIMER_EDIT_TRANSITION_TIMEOUT_MS, KITCHENTIMER_EDIT_HOLD_TIMEOUT_MS);
	    digital_timer->enterEditMode (KITCHENTIMER_EDIT_TRANSITION_TIMEOUT_MS);
	    startShading (KITCHENTIMER_EDIT_TRANSITION_TIMEOUT_MS);
	} else {
	    analog_timer->enterEditModePressed (KITCHENTIMER_INITIAL_EDIT_TRANSITION_TIMEOUT_MS, KITCHENTIMER_INITIAL_EDIT_HOLD_TIMEOUT_MS);
	    digital_timer->enterEditMode (KITCHENTIMER_INITIAL_EDIT_TRANSITION_TIMEOUT_MS);
	    startShading (KITCHENTIMER_INITIAL_EDIT_TRANSITION_TIMEOUT_MS);
	    app_manager->edition_happened = true;
	}
	app_manager->getCurrentTimer ()->stop ();
	// leave_edit_mode_timer.start (KITCHENTIMER_LEAVE_EDIT_MODE_TIMEOUT_MS);
    }
}
void PageTimers::leaveEditMode ()
{
    if (edit_mode) {
	edit_mode = false;
	app_manager->clearAlarms ();
	analog_timer->leaveEditMode ();
	digital_timer->leaveEditMode ();
	setShaded (false);

	Timer *current_timer = app_manager->getCurrentTimer ();
	if (QTime (0, 0, 0).secsTo (current_timer->getTimeLeft ())) {
	    current_timer->start ();
	    app_manager->runTimerStart ();
	}
	leave_edit_mode_timer.stop ();
    }
}
void PageTimers::timeout ()
{
    app_manager->runAlarmOnce ();
}
void PageTimers::restartTirednessTimer ()
{
    if (leave_edit_mode_timer.isActive ())
	leave_edit_mode_timer.start (KITCHENTIMER_LEAVE_EDIT_MODE_TIMEOUT_MS);
}
void PageTimers::startTirednessTimer ()
{
    leave_edit_mode_timer.start (KITCHENTIMER_LEAVE_EDIT_MODE_TIMEOUT_MS);
}
