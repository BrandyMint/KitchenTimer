#include "timer.h"


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
    if (running) {
	qint64 period_ms = QTime (0, 0, 0).msecsTo (period);
	qint64 elapsed_ms = elapsed_timer.elapsed ();
	return QTime (0, 0, 0).addMSecs (qMax (period_ms - elapsed_ms, qint64 (0)));
    } else {
	return time_left;
    }
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
