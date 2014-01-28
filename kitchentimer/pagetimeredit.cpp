#include "pagetimeredit.h"
#include "timer.h"
#include "application.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>
#include <QSpinBox>
#include <QDial>


PageTimerEdit::PageTimerEdit (QWidget *parent)
    : QWidget (parent)
{
    QVBoxLayout *layout = new QVBoxLayout (this);
    layout->addStretch (1);
    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	hlayout->addStretch (1);
	title_line_edit = new QLineEdit (this);
	hlayout->addWidget (title_line_edit);
	hlayout->addStretch (1);
	layout->addLayout (hlayout);
    }
    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	hlayout->addStretch (1);
	hours_spin_box = new QSpinBox (this);
	hours_spin_box->setRange (0, 23);
	hlayout->addWidget (hours_spin_box);
	hlayout->addWidget (new QLabel (":", this));
	minutes_spin_box = new QSpinBox (this);
	minutes_spin_box->setRange (0, 59);
	hlayout->addWidget (minutes_spin_box);
	hlayout->addWidget (new QLabel (":", this));
	seconds_spin_box = new QSpinBox (this);
	seconds_spin_box->setRange (0, 59);
	hlayout->addWidget (seconds_spin_box);
	hlayout->addStretch (1);
	layout->addLayout (hlayout);
    }
    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	hlayout->addStretch (1);
	QDial *dial = new QDial (this);
	dial->setMinimumSize (QSize (200, 200));
	hlayout->addWidget (dial);
	hlayout->addStretch (1);
	layout->addLayout (hlayout);
    }
    layout->addStretch (1);
    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	QPushButton *cancel_button = new QPushButton ("Cancel", this);
	connect (cancel_button, SIGNAL (clicked ()), this, SIGNAL (cancel ()));
	hlayout->addWidget (cancel_button);
	hlayout->addStretch (1);
	QPushButton *accept_button = new QPushButton ("Accept", this);
	connect (accept_button, SIGNAL (clicked ()), this, SLOT (acceptTimer ()));
	hlayout->addWidget (accept_button);
	layout->addLayout (hlayout);
    }
}
PageTimerEdit::~PageTimerEdit ()
{
}
void PageTimerEdit::updateContent ()
{
    QList<Timer*> &timers = app->getTimers ();
    int current_timer_index = app->getCurrentTimerIndex ();
    if (!timers.size () || current_timer_index < 0 || current_timer_index >= timers.size ()) {
	title_line_edit->setText ("");
	return;
    }
    Timer *current_timer = timers[current_timer_index];
    title_line_edit->setText (current_timer->getTitle ());
    const QTime &period = current_timer->getPeriod ();
    hours_spin_box->setValue (period.hour ());
    minutes_spin_box->setValue (period.minute ());
    seconds_spin_box->setValue (period.second ());
}
void PageTimerEdit::acceptTimer ()
{
    QString title = title_line_edit->text ();
    QTime period = QTime (0, 0, 0);
    period.setHMS (hours_spin_box->value (), minutes_spin_box->value (), seconds_spin_box->value ());
    emit accept (title, period);
}
