#include "timer.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include <stdio.h>


Timer::Timer (const QTime &time_left, const QString &title)
    : period (0, 0, 0), time_left (time_left), title (title), running (false)
{
    elapsed_timer.invalidate ();
    connect (&main_timer, SIGNAL (timeout ()), this, SLOT (internalTimeout ()));
    connect (&ticker, SIGNAL (timeout ()), this, SLOT (internalTick ()));
}
const QTime &Timer::getPeriod ()
{
    return period;
}
void Timer::setTimeLeft (const QTime &new_time_left)
{
    time_left = new_time_left;
    emit newTimeSet ();
}
QTime Timer::getTimeLeft ()
{
    if (running)
	return QTime (0, 0, 0).addMSecs ((QTime (0, 0, 0).msecsTo (period)) - elapsed_timer.elapsed ());
    else
	return time_left;
}
void Timer::start ()
{
    main_timer.setSingleShot (true);
    main_timer.start (QTime (0, 0, 0).msecsTo (time_left));
    period = time_left;
    elapsed_timer.start ();
    running = true;
    ticker.start (100);
}
void Timer::stop ()
{
    main_timer.stop ();
    elapsed_timer.invalidate ();
    ticker.stop ();
    running = false;
}
bool Timer::isRunning ()
{
    return running;
}
void Timer::setTitle (const QString &new_title)
{
    title = new_title;
}
const QString &Timer::getTitle ()
{
    return title;
}
void Timer::internalTick ()
{
    if (QTime (0, 0, 0).msecsTo (time_left) < 100) {
	time_left = QTime (0, 0, 0);
    } else {
	time_left = time_left.addMSecs (-100);
    }
    emit updateTick ();
}
void Timer::internalTimeout ()
{
    ticker.stop ();
    time_left = QTime (0, 0, 0);
    running = false;
    emit timeout ();
}
