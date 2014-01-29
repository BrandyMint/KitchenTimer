#include "pagetimers.h"
#include "application.h"
#include "customdial.h"
#include "clickablelabel.h"

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

#include <stdio.h>


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
    : Background (parent)
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
	    QHBoxLayout *hlayout = new QHBoxLayout ();
	    previous_timer_button = new QPushButton (app->previous_timer_icon, "", current_timer_subpage);
	    connect (previous_timer_button, SIGNAL (clicked ()), this, SIGNAL (previousTimer ()));
	    hlayout->addWidget (previous_timer_button);
	    hlayout->addStretch (1);
	    current_timer_dial = new CustomDial (current_timer_subpage);
	    connect (current_timer_dial, SIGNAL (valueChanged (int)), this, SLOT (dialValueChanged (int)));
	    connect (current_timer_dial, SIGNAL (sliderPressed ()), this, SIGNAL (stopCurrentTimer ()));
	    connect (current_timer_dial, SIGNAL (sliderReleased ()), this, SLOT (currentTimerAdjusted ()));
	    hlayout->addWidget (current_timer_dial, 40);
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
	}
	{
	    current_timer_time_left_label = new ClickableLabel (current_timer_subpage);
	    current_timer_time_left_label->setFont (app->getBigFont ());
	    current_timer_time_left_label->setStyleSheet ("QLabel { color : #fffffd; }");
	    current_timer_time_left_label->setAlignment (Qt::AlignHCenter | Qt::AlignVCenter);
	    subpage_layout->addWidget (current_timer_time_left_label);
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
    QList<Timer*> &timers = app->getTimers ();
    int current_timer_index = app->getCurrentTimerIndex ();
    if (!timers.size () || current_timer_index < 0 || current_timer_index >= timers.size ()) {
	return;
    }
    if (!current_timer_dial->isSliderDown ()) {
	Timer *current_timer = timers[current_timer_index];
	current_timer_title_label->setText (current_timer->getTitle ());
	current_timer_time_left_label->setText (current_timer->getTimeLeft ().toString ());
	previous_timer_button->setEnabled (current_timer_index > 0);
	next_timer_button->setEnabled (current_timer_index < (timers.count () - 1));
	current_timer_dial->setValue (QTime (0, 0, 0).secsTo (current_timer->getTimeLeft ()));
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
    if (current_timer_dial->isSliderDown ()) {
	app->clearAlarms ();
    }
    QList<Timer*> &timers = app->getTimers ();
    int current_timer_index = app->getCurrentTimerIndex ();
    if (!timers.size () || current_timer_index < 0 || current_timer_index >= timers.size ()) {
	return;
    }
    Timer *current_timer = timers[current_timer_index];
    dial_value = QTime (0, 0, 0).addSecs (new_period_sec);
    current_timer_time_left_label->setText (dial_value.toString ("hh:mm:ss"));
}
void PageTimers::currentTimerAdjusted ()
{
    emit setStartCurrentTimer (dial_value);
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
