#include "pagetimers.h"
#include "application.h"
#include "widgets/analogtimer.h"
#include "widgets/digitaltimer.h"
#include "widgets/clickablelabel.h"

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


TimerLabel::TimerLabel (QWidget *parent, Timer *timer)
    : QLabel (parent), timer (timer)
{
    connect (timer, SIGNAL (updateTick ()), this, SLOT (updateContent ()));
    connect (timer, SIGNAL (timeout ()), this, SLOT (timeout ()));
}
void TimerLabel::updateContent ()
{
    setText ("<p align=\"center\">" + timer->getTitle ().toHtmlEscaped () + "<br />" +
	     timer->getTimeLeft ().toString () + "/" +
	     timer->getPeriod ().toString () + "</p>");
}
void TimerLabel::timeout ()
{
    updateContent ();
    app->runAlarmOnce ();
}


PageTimers::PageTimers (QWidget *parent)
    : Background (parent), edit_mode (false)
{
    QVBoxLayout *layout = new QVBoxLayout (this);

    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	toggle_audio_button = new QPushButton (app->getAudioEnabled () ? app->audio_enabled_icon : app->audio_disabled_icon,
					       "", this);
	connect (toggle_audio_button, SIGNAL (clicked ()), app, SLOT (toggleAudioEnabled ()));
	hlayout->addWidget (toggle_audio_button);
	toggle_vibrosignal_button = new QPushButton (app->getVibrosignalEnabled () ? app->vibrosignal_enabled_icon : app->vibrosignal_disabled_icon,
						     "", this);
	connect (toggle_vibrosignal_button, SIGNAL (clicked ()), app, SLOT (toggleVibrosignalEnabled ()));
	hlayout->addWidget (toggle_vibrosignal_button);
	hlayout->addStretch (1);
	QPushButton *bookmarks_button = new QPushButton (app->bookmarks_icon, "", this);
	hlayout->addWidget (bookmarks_button);
	layout->addLayout (hlayout);
#if 1 // Temporarily hide
	toggle_audio_button->hide ();
	toggle_vibrosignal_button->hide ();
	bookmarks_button->hide ();
#endif
    }
    
    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	hlayout->addStretch (1);
	goto_timer_button = new QPushButton (app->current_timer_icon, "", this);
	connect (goto_timer_button, SIGNAL (clicked ()), this, SLOT (setSubpageCurrentTimer ()));
	hlayout->addWidget (goto_timer_button);
	goto_timer_list_button = new QPushButton (app->timer_list_icon, "", this);
	connect (goto_timer_list_button, SIGNAL (clicked ()), this, SLOT (setSubpageTimerList ()));
	hlayout->addWidget (goto_timer_list_button);
	hlayout->addStretch (1);
	layout->addLayout (hlayout);
#if 1 // Temporarily hide
	goto_timer_button->hide ();
	goto_timer_list_button->hide ();
#endif
    }
    
    layout->addSpacing (10);

    stacked_widget = new QStackedWidget (this);
    { // Current timer subpage
	current_timer_subpage = new QWidget (stacked_widget);
	QVBoxLayout *subpage_layout = new QVBoxLayout (current_timer_subpage);
	subpage_layout->addStretch (1);
	{
	    digital_timer = new DigitalTimer (current_timer_subpage);
	    connect (digital_timer, SIGNAL (enterEditModeRequested ()), this, SLOT (enterEditMode ()));
	    subpage_layout->addWidget (digital_timer);
	}

	{
	    QHBoxLayout *hlayout = new QHBoxLayout ();
	    previous_timer_button = new QPushButton (app->previous_timer_icon, "", current_timer_subpage);
	    connect (previous_timer_button, SIGNAL (clicked ()), this, SIGNAL (previousTimer ()));
	    hlayout->addWidget (previous_timer_button);
	    hlayout->addStretch (1);
	    analog_timer = new AnalogTimer (current_timer_subpage);
	    connect (analog_timer, SIGNAL (timeChanged (const QTime&)), this, SLOT (analogTimeChanged (const QTime&)));
	    connect (analog_timer, SIGNAL (clearAlarms ()), this, SLOT (clearCurrentAlarms ()));
	    connect (analog_timer, SIGNAL (enterEditModeRequested ()), this, SLOT (enterEditMode ()));
	    connect (analog_timer, SIGNAL (leaveEditModeRequested ()), this, SLOT (leaveEditMode ()));
	    hlayout->addWidget (analog_timer, 40);
	    next_timer_button = new QPushButton (app->next_timer_icon, "", current_timer_subpage);
	    connect (next_timer_button, SIGNAL (clicked ()), this, SIGNAL (nextTimer ()));
	    hlayout->addWidget (next_timer_button);
	    hlayout->addStretch (1);
	    subpage_layout->addLayout (hlayout, 1);
#if 1 // Temporarily hide
	    previous_timer_button->hide ();
	    next_timer_button->hide ();
#endif
	}

	{
	    current_timer_title_label = new ClickableLabel (current_timer_subpage);
	    current_timer_title_label->setStyleSheet ("QLabel { color : #fcd2a8; }");
	    current_timer_title_label->setAlignment (Qt::AlignHCenter | Qt::AlignVCenter);
	    subpage_layout->addWidget (current_timer_title_label);
#if 1 // Temporarily hide
	    current_timer_title_label->hide ();
#endif
	}

	subpage_layout->addStretch (1);

	stacked_widget->addWidget (current_timer_subpage);
    }
    { // Timer list subpage
	// TODO: QScroller *scroller = QScroller::scroller (scrolled_area);

	scroll_area = new QScrollArea (stacked_widget);
	scroll_area->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
	scroll_area->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
	scroll_widget = new QWidget (scroll_area);
	scroll_layout = new QVBoxLayout (scroll_widget);
	scroll_area->setWidget (scroll_widget);
	scroll_area->setWidgetResizable (true);

	stacked_widget->addWidget (scroll_area);
    }
    layout->addWidget (stacked_widget, 1);
    
    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	hlayout->addStretch (1);
	QPushButton *add_timer_button = new QPushButton ("Add timer", this);
	connect (add_timer_button, SIGNAL (clicked ()), this, SIGNAL (switchToPageDishSelect ()));
	hlayout->addWidget (add_timer_button);
	hlayout->addStretch (1);
	layout->addLayout (hlayout);
#if 1 // Temporarily hide
	add_timer_button->hide ();
#endif
    }
    
    connect (app, SIGNAL (valueChangedAudioEnabled (bool)), this, SLOT (setAudioEnabled (bool)));
    connect (app, SIGNAL (valueChangedVibrosignalEnabled (bool)), this, SLOT (setVibrosignalEnabled (bool)));
    connect (this, SIGNAL (pressed ()), this, SLOT (leaveEditMode ()));

    updateContent ();
    setSubpageCurrentTimer ();

    connect (&update_timer, SIGNAL (timeout ()), this, SLOT (updateContentSubpageCurrentTimer ()));
    update_timer.setInterval (250);
    update_timer.setSingleShot (false);
    update_timer.start ();
}
PageTimers::~PageTimers ()
{
}
void PageTimers::updateContentSubpageCurrentTimer ()
{
    if (!analog_timer->isSliderDown ()) {
	Timer *current_timer = app->getCurrentTimer ();
	QList<Timer*> &timers = app->getTimers ();
	int current_timer_index = app->getCurrentTimerIndex ();
	current_timer_title_label->setText (current_timer->getTitle ());
	digital_timer->setTime (current_timer->getTimeLeft ());
	previous_timer_button->setEnabled (current_timer_index > 0);
	next_timer_button->setEnabled (current_timer_index < (timers.count () - 1));
	analog_timer->setTime (current_timer->getTimeLeft ());
    }
}
void PageTimers::updateContentSubpageTimerList ()
{
    QList<Timer*> &timers = app->getTimers ();
    delete scroll_widget;
    scroll_widget = new QWidget (scroll_area);
    scroll_layout = new QVBoxLayout (scroll_widget);

    scroll_layout->addStretch (1);

    if (!timers.count ()) {
	QLabel *label = new QLabel ("No timers", scroll_widget);
	scroll_layout->addWidget (label);
    } else {
	QPushButton *button;
	int i = 0;
	for (QList<Timer*>::iterator it = timers.begin (); it != timers.end (); ++it, ++i) {
	    if (it != timers.begin ())
		scroll_layout->addSpacing (10);
	    Timer *timer = *it;
	    const QString &title = timer->getTitle ();
	    QHBoxLayout *hlayout = new QHBoxLayout ();
	    hlayout->addStretch (1);
	    TimerLabel *label = new TimerLabel (scroll_widget, timer);
	    label->updateContent ();
	    hlayout->addWidget (label);
	    hlayout->addWidget (button = new QPushButton ("Stop/Play", scroll_widget));
	    button->setProperty ("timer_index", i);
	    connect (button, SIGNAL (clicked ()), this, SLOT (timerStartStop ()));
	    hlayout->addWidget (button = new QPushButton ("Reset", scroll_widget));
	    button->setProperty ("timer_index", i);
	    connect (button, SIGNAL (clicked ()), this, SLOT (timerReset ()));
	    hlayout->addWidget (button = new QPushButton ("Edit", scroll_widget));
	    button->setProperty ("timer_index", i);
	    connect (button, SIGNAL (clicked ()), this, SLOT (timerEdit ()));
	    hlayout->addWidget (button = new QPushButton ("Select", scroll_widget));
	    button->setProperty ("timer_index", i);
	    connect (button, SIGNAL (clicked ()), this, SLOT (timerSelect ()));
	    hlayout->addWidget (button = new QPushButton ("Remove", scroll_widget));
	    button->setProperty ("timer_index", i);
	    connect (button, SIGNAL (clicked ()), this, SLOT (timerRemove ()));
	    hlayout->addWidget (button = new QPushButton ("Move", scroll_widget));
	    button->setProperty ("timer_index", i);
	    connect (button, SIGNAL (clicked ()), this, SLOT (timerMove ()));
	    hlayout->addWidget (button = new QPushButton ("Bookmark", scroll_widget));
	    button->setProperty ("timer_index", i);
	    connect (button, SIGNAL (clicked ()), this, SLOT (timerBookmark ()));
	    hlayout->addStretch (1);
	    scroll_layout->addLayout (hlayout);
	}
    }

    scroll_layout->addStretch (1);

    scroll_area->setWidget (scroll_widget);
}
void PageTimers::updateContent ()
{
    updateContentSubpageCurrentTimer ();
    updateContentSubpageTimerList ();
}
void PageTimers::setAudioEnabled (bool new_value)
{
    toggle_audio_button->setIcon (new_value ? app->audio_enabled_icon : app->audio_disabled_icon);
}
void PageTimers::setVibrosignalEnabled (bool new_value)
{
    toggle_vibrosignal_button->setIcon (new_value ? app->vibrosignal_enabled_icon : app->vibrosignal_disabled_icon);
}
void PageTimers::setSubpageCurrentTimer ()
{
    updateContentSubpageCurrentTimer ();
    stacked_widget->setCurrentWidget (current_timer_subpage);
    goto_timer_button->setEnabled (false);
    goto_timer_list_button->setEnabled (true);
}
void PageTimers::setSubpageTimerList ()
{
    updateContentSubpageTimerList ();
    stacked_widget->setCurrentWidget (scroll_area);
    goto_timer_button->setEnabled (true);
    goto_timer_list_button->setEnabled (false);
}
void PageTimers::dialValueChanged (int new_period_sec)
{
    dial_value = QTime (0, 0, 0).addSecs (new_period_sec);
    Timer *current_timer = app->getCurrentTimer ();
    digital_timer->setTime (dial_value);
}
void PageTimers::analogTimeChanged (const QTime &new_time)
{
    dial_value = new_time;
    digital_timer->setTime (dial_value);
    currentTimerAdjusted ();
}
void PageTimers::currentTimerAdjusted ()
{
    // emit setStartCurrentTimer (dial_value);
    Timer *current_timer = app->getCurrentTimer ();
    current_timer->setPeriod (dial_value);
    current_timer->setTimeLeft (dial_value);
}
void PageTimers::timerStartStop ()
{
    QList<Timer*> &timers = app->getTimers ();
    Timer *timer = timers[sender ()->property ("timer_index").toInt ()];
    if (timer->isRunning ()) {
	timer->stop ();
    } else {
	timer->start ();
    }
}
void PageTimers::timerReset ()
{
    QList<Timer*> &timers = app->getTimers ();
    Timer *timer = timers[sender ()->property ("timer_index").toInt ()];
    timer->setTimeLeft (timer->getPeriod ());
    updateContentSubpageTimerList ();
}
void PageTimers::timerEdit ()
{
    QList<Timer*> &timers = app->getTimers ();
    Timer *timer = timers[sender ()->property ("timer_index").toInt ()];
    app->setCurrentTimerIndex (sender ()->property ("timer_index").toInt ());
    emit editCurrentTimer ();
}
void PageTimers::timerSelect ()
{
    app->setCurrentTimerIndex (sender ()->property ("timer_index").toInt ());
    setSubpageCurrentTimer ();
}
void PageTimers::timerRemove ()
{
    emit removeTimer (sender ()->property ("timer_index").toInt ());
}
void PageTimers::timerMove ()
{
    QList<Timer*> &timers = app->getTimers ();
    Timer *timer = timers[sender ()->property ("timer_index").toInt ()];
}
void PageTimers::timerBookmark ()
{
    QList<Timer*> &timers = app->getTimers ();
    Timer *timer = timers[sender ()->property ("timer_index").toInt ()];
}
void PageTimers::clearCurrentAlarms ()
{
    app->clearAlarms ();
}
void PageTimers::switchEditMode (bool new_edit_mode)
{
    edit_mode = new_edit_mode;
    app->clearAlarms ();
    analog_timer->setEditMode (new_edit_mode);
    digital_timer->setEditMode (new_edit_mode);
    setShaded (new_edit_mode);

    Timer *current_timer = app->getCurrentTimer ();
    if (new_edit_mode) {
	saved_period_value = current_timer->getPeriod ();
	saved_time_left_value = current_timer->getTimeLeft ();
	saved_running_state = current_timer->isRunning ();
	current_timer->stop ();
    } else {
	// if (saved_running_state)
	current_timer->start ();
    }
}
void PageTimers::restoreLeaveEditMode ()
{
    Timer *current_timer = app->getCurrentTimer ();
    current_timer->setPeriod (saved_period_value);
    current_timer->setTimeLeft (saved_time_left_value);
    switchEditMode (false);
}
void PageTimers::enterEditMode ()
{
    stopCurrentTimer ();
    if (!edit_mode)
	switchEditMode (true);
}
void PageTimers::leaveEditMode ()
{
    if (edit_mode)
	switchEditMode (false);
}
