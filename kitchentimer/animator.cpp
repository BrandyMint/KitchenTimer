#include "animator.h"

#include <QWidget>
#if 1 // TODO: Temporary debug stuff
#include <QTime>
#include <QDebug>
#endif

Animator::Animator (QWidget *widget, int min_frame_timeout)
    : widget (widget), min_frame_timeout (min_frame_timeout), running (false), duration_ms (-1)
{
    elapsed_timer.invalidate ();
    timer.setSingleShot (true);
    connect (&timer, SIGNAL (timeout ()), this, SLOT (iterateUpdate ()));
}
void Animator::start (int new_durations_ms)
{
    running = true;
    timer.start (min_frame_timeout);
    if (new_durations_ms > 0) {
	duration_ms = new_durations_ms;
	elapsed_timer.start ();
    } else {
	duration_ms = -1;
	elapsed_timer.invalidate ();
    }
}
void Animator::stop ()
{
    running = false;
    duration_ms = -1;
    elapsed_timer.invalidate ();
    widget->update ();
    emit stopped ();
}
bool Animator::isRunning ()
{
    return running;
}
double Animator::phase ()
{
    if (running && elapsed_timer.isValid () && (duration_ms > 0))
	return qMin (double (elapsed_timer.elapsed ())/double (duration_ms), 1.0);
    return 1.0;
}
void Animator::setMinFrameTimeout (int new_min_frame_timeout)
{
    min_frame_timeout = new_min_frame_timeout;
}
void Animator::iterateUpdate ()
{
    if (running) {
#if 0 // TODO: Temporary debug stuff
	qWarning ("(%p) UPDATE: %d", this, QTime::currentTime ().msec ());
#endif
	if (elapsed_timer.isValid () && (elapsed_timer.elapsed () >= duration_ms)) {
	    stop ();
	} else {
	    widget->update ();
	    timer.start (min_frame_timeout);
	}
    } else {
	widget->update ();
    }
}
