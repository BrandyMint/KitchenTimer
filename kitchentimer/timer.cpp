#include "timer.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include <stdio.h>


Timer::Timer (const QTime &time_left, const QString &title)
    : period (), time_left (time_left), title (title), running (false)
{
    main_timer.setSingleShot (true);
    elapsed_timer.invalidate ();
    connect (&main_timer, SIGNAL (timeout ()), this, SLOT (internalTimeout ()));
}
const QTime &Timer::getPeriod ()
{
    return period;
}
void Timer::setTimeLeft (const QTime &new_time_left)
{
    time_left = new_time_left;
    period = QTime ();
    emit newTimeSet ();
}
QTime Timer::getTimeLeft ()
{
    if (running)
	return QTime (0, 0, 0).addMSecs ((QTime (0, 0, 0).msecsTo (period)) - elapsed_timer.elapsed ());
    else
	return time_left;
}
int Timer::getMSElapsed ()
{
    if (running)
	return elapsed_timer.elapsed ();
    else
	return 0;
}
void Timer::start ()
{
    main_timer.start (QTime (0, 0, 0).msecsTo (time_left));
    period = time_left;
    elapsed_timer.start ();
    running = true;
}
void Timer::stop ()
{
    main_timer.stop ();
    if (running)
	time_left = QTime (0, 0, 0).addMSecs ((QTime (0, 0, 0).msecsTo (period)) - elapsed_timer.elapsed ());
    elapsed_timer.invalidate ();
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
void Timer::internalTimeout ()
{
    time_left = QTime (0, 0, 0);
    running = false;
    emit timeout ();
}
