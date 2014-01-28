#include "timer.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include <stdio.h>


Timer::Timer (const QTime &period, const QTime &time_left, const QString &title)
    : period (period), time_left (time_left), title (title), running (false)
{
    connect (&main_timer, SIGNAL (timeout ()), this, SLOT (internalTimeout ()));
    connect (&ticker, SIGNAL (timeout ()), this, SLOT (internalTick ()));
}
void Timer::setPeriod (const QTime &new_period)
{
    period = new_period;
}
const QTime &Timer::getPeriod ()
{
    return period;
}
void Timer::setTimeLeft (const QTime &new_time_left)
{
    time_left = new_time_left;
}
const QTime &Timer::getTimeLeft ()
{
    return time_left;
}
void Timer::start ()
{
    main_timer.setSingleShot (true);
    main_timer.start (QTime (0, 0, 0).msecsTo (time_left));
    running = true;
    ticker.start (1000);
}
void Timer::stop ()
{
    main_timer.stop ();
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
    time_left = time_left.addSecs (-1);
    if (QTime (0, 0, 0).msecsTo (time_left) < 0)
	time_left = QTime (0, 0, 0);
    emit updateTick ();
}
void Timer::internalTimeout ()
{
    ticker.stop ();
    time_left = QTime (0, 0, 0);
    running = false;
    emit timeout ();
}
