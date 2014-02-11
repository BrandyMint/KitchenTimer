#include "editmode.h"


EditMode::EditMode ()
    : is_enabled (false), is_blocked (false), halted (false), halted_by_timeout (false)
{
    unblock_timer.setSingleShot (true);
    unhalt_timer.setSingleShot (true);
    connect (&unblock_timer, SIGNAL (timeout ()), this, SLOT (unblockEdit ()));
    connect (&unhalt_timer, SIGNAL (timeout ()), this, SLOT (unhaltByTimeout ()));
}
bool EditMode::isEnabled ()
{
    return is_enabled;
}
bool EditMode::isBlocked ()
{
    return is_blocked;
}
bool EditMode::isHalted ()
{
    return halted;
}
bool EditMode::isHaltedByTimeout ()
{
    return halted && halted_by_timeout;
}
void EditMode::halt ()
{
    unhalt_timer.stop ();
    halted = true;
    halted_by_timeout = false;
}
void EditMode::unhalt ()
{
    unhalt_timer.stop ();
    halted = false;
    halted_by_timeout = false;
}
void EditMode::enter (int unblock_timeout)
{
    unblock_timer.stop ();
    unhalt_timer.stop ();
    is_enabled = true;
    if (unblock_timeout > 0) {
	is_blocked = true;
	unblock_timer.start (unblock_timeout);
    } else {
	is_blocked = false;
    }
    halted = false;
    halted_by_timeout = false;
}
void EditMode::enterPressed (int unblock_timeout, int unhalt_timeout)
{
    unblock_timer.stop ();
    unhalt_timer.stop ();
    is_enabled = true;
    if (unblock_timeout > 0) {
	is_blocked = true;
	halted = true;
	halted_by_timeout = true;
	unblock_timer.start (unblock_timeout);
	unhalt_timer.start (unhalt_timeout);
    } else {
	is_blocked = false;
	halted = false;
	halted_by_timeout = false;
    }
}
void EditMode::leave ()
{
    unblock_timer.stop ();
    unhalt_timer.stop ();
    is_enabled = false;
    is_blocked = false;
    halted = false;
    halted_by_timeout = false;
}
void EditMode::unblockEdit ()
{
    is_blocked = false;
    emit unblockedByTimeout ();
}
void EditMode::unhaltByTimeout ()
{
    if (is_enabled && halted && halted_by_timeout) {
    	halted = false;
    	halted_by_timeout = false;
	emit unhaltedByTimeout ();
    } else {
    	halted = false;
    	halted_by_timeout = false;
    }
}
