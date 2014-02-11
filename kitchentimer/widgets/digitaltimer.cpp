#include <QPainter>
#include <QMouseEvent>
#include <QtCore/qmath.h>
#include <QApplication>
#include <QPicture>
#include <QPropertyAnimation>
#include <QDebug>

#include "digitaltimer.h"
#include "applicationmanager.h"

#define MAX_FONT_SIZE 600
#define MIN_FONT_SIZE 4

#define MAX_MIN (6*60 - 1)
#define MAX_SEC (MAX_MIN*60)
#define MAX_MSEC (MAX_SEC*1000)

#define MIN_OFFSET_TO_START_SCROLL_FRACTION 0.2
#define MIN_OFFSET_TO_START_SCROLL_MIN_PIXELS 10
#define SCROLL_STEP_FRACTION 0.05
#define SCROLL_STEP_MIN_PIXELS 3

#define TEXT_FLAGS (Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip | Qt::TextIncludeTrailingSpaces)

#define ANIMATION_MIN_FRAME_TIMEOUT 10


DigitalTimer::DigitalTimer (QWidget *parent)
    : QWidget (parent),
      font (qApp->font ()), font2 (qApp->font ()),
      animator (this, ANIMATION_MIN_FRAME_TIMEOUT),
      unblock_timeout (0), down (false), button_pressed (NonePressed), button_pressed_rect (0, 0, 0, 0), button_pressed_point (-1000000, -1000000),
      estimated_size (-1, -1),
      estimated_font_size (3),
      estimated_char_bounding_size (-1, -1),
      estimated_arrow_bounding_size (-1, -1),
      estimated_separator_bounding_size (-1, -1),
      estimated_min_scroll_offset (MIN_OFFSET_TO_START_SCROLL_MIN_PIXELS),
      estimated_scroll_step_offset (SCROLL_STEP_MIN_PIXELS),
      estimated_minute_add_rect (0, 0, 0, 0),
      estimated_minute_subtract_rect (0, 0, 0, 0),
      estimated_minute_scroll_rect (0, 0, 0, 0)
{
    font.setPixelSize (estimated_font_size);
    setFont (font);
    connect (app_manager->getCurrentTimer (), SIGNAL (timeout ()), this, SLOT (update ()));
    connect (app_manager->getCurrentTimer (), SIGNAL (timeout ()), &animator, SLOT (stop ()));
    connect (app_manager->getCurrentTimer (), SIGNAL (newTimeSet ()), this, SLOT (update ()));
    connect (&edit_mode, SIGNAL (unblockedByTimeout ()), this, SLOT (update ()));
    connect (&edit_mode, SIGNAL (unhaltedByTimeout ()), this, SLOT (unhaltEditByTimeout ()));
}
DigitalTimer::~DigitalTimer ()
{
}
bool DigitalTimer::addMinuteSharp ()
{
    Timer *timer = app_manager->getCurrentTimer ();
    QTime value = timer->getTimeLeft ();
    int ms = QTime (0, 0, 0).msecsTo (value); // TODO: Switch to Qt-5.2.0: .msecsSinceStartOfDay ();
    if (ms == (MAX_SEC*1000))
	return false;
    if (ms%60000)
	ms = ms - ms%60000 + 60000;
    ms += 60000;
    if (ms > (MAX_SEC*1000))
	ms = MAX_SEC*1000;
    value = QTime (0, 0, 0).addMSecs (ms); // TODO: Switch to Qt-5.2.0: QTime::fromMSecsSinceStartOfDay (ms);
    timer->setTimeLeft (value);
    return true;
}
bool DigitalTimer::subtractMinuteSharp ()
{
    Timer *timer = app_manager->getCurrentTimer ();
    QTime value = timer->getTimeLeft ();
    int ms = QTime (0, 0, 0).msecsTo (value); // TODO: Switch to Qt-5.2.0: .msecsSinceStartOfDay ();
    if (ms == 0)
	return false;
    if (ms%60000)
	ms = ms - ms%60000 + 60000;
    ms -= 60000;
    if (ms < 0)
	ms = 0;
    value = QTime (0, 0, 0).addMSecs (ms); // TODO: Switch to Qt-5.2.0: QTime::fromMSecsSinceStartOfDay (ms);
    timer->setTimeLeft (value);
    return true;
}
void DigitalTimer::enterEditMode (int new_unblock_timeout)
{
    unblock_timeout = new_unblock_timeout;
    edit_mode.enter (new_unblock_timeout);
    animator.stop ();
    update ();
}
void DigitalTimer::enterEditModePressed (int new_unblock_timeout, int new_unhalt_timeout)
{
    unblock_timeout = new_unblock_timeout;
    edit_mode.enterPressed (new_unblock_timeout, new_unhalt_timeout);
    animator.stop ();
    update ();
}
void DigitalTimer::leaveEditMode ()
{
    edit_mode.leave ();
    int ms = QTime (0, 0, 0).msecsTo (app_manager->getCurrentTimer ()->getTimeLeft ()); // TODO: Switch to Qt-5.2.0: .msecsSinceStartOfDay ();
    if (ms > 0)
	animator.start ();
    update ();
}
void DigitalTimer::unhaltEditByTimeout ()
{
    if (down && edit_mode.isEnabled ()) {
	if (!edit_mode.isBlocked ()) {
	    if (estimated_minute_add_rect.contains (button_pressed_point)) {
		button_pressed = AddMinutePressed;
		button_pressed_rect = estimated_minute_add_rect;
	    } else if (estimated_minute_subtract_rect.contains (button_pressed_point)) {
		button_pressed = SubtractMinutePressed;
		button_pressed_rect = estimated_minute_subtract_rect;
	    } else if (estimated_minute_scroll_rect.contains (button_pressed_point)) {
		button_pressed = ScrollMinutePressed;
		button_pressed_rect = rect ();
		vertical_scroll_accum = 0;
	    } else {
		button_pressed = LeaveAreaPressed;
		button_pressed_rect = estimated_minute_scroll_rect;
	    }
	}
	previous_point = button_pressed_point;
	update ();
    }
}
void DigitalTimer::resizeEvent (QResizeEvent*)
{
    switch (KITCHENTIMER_DIGITAL_TIMER_MODE) {
    case 0: {
	QPainter p (&estimation_picture);
	p.setFont (font);
	const QRect &current_rect = rect ();
	QSize current_size = current_rect.size ();
	if (current_size != estimated_size) {
	    int w = current_size.width ();
	    int h = current_size.height ();
	    QSize char_bounding_size = charBoundingSize (p);
	    QSize arrow_bounding_size = arrowBoundingSize (p);
	    QSize bounding_size = QSize (char_bounding_size.width ()*8, char_bounding_size.height ()*2 + arrow_bounding_size.width ()*2);
	    if ((bounding_size.width () <= w) && (bounding_size.height () <= h)) {
		// Increasing size
		while (estimated_font_size < MAX_FONT_SIZE) {
		    font.setPixelSize (estimated_font_size + 1);
		    p.setFont (font);
		    char_bounding_size = charBoundingSize (p);
		    arrow_bounding_size = arrowBoundingSize (p);
		    bounding_size = QSize (char_bounding_size.width ()*8, char_bounding_size.height ()*2 + arrow_bounding_size.width ()*2);
		    if ((bounding_size.width () > current_size.width ()) ||
			(bounding_size.height () > current_size.height ()))
			break;
		    ++estimated_font_size;
		}
	    } else {
		// Decreasing size
		while (estimated_font_size > MIN_FONT_SIZE) {
		    font.setPixelSize (estimated_font_size);
		    p.setFont (font);
		    char_bounding_size = charBoundingSize (p);
		    arrow_bounding_size = arrowBoundingSize (p);
		    bounding_size = QSize (char_bounding_size.width ()*8, char_bounding_size.height ()*2 + arrow_bounding_size.width ()*2);
		    if ((bounding_size.width () <= w) && (bounding_size.height () <= h))
			break;
		    --estimated_font_size;
		}
	    }
	    font.setPixelSize (estimated_font_size);
	    p.setFont (font);
	    setFont (font);
	    estimated_size = current_size;
	    estimated_char_bounding_size = charBoundingSize (p);
	    estimated_arrow_bounding_size = arrowBoundingSize (p);
	    estimated_separator_bounding_size = separatorBoundingSize (p);
	    estimated_min_scroll_offset = qMax (int (MIN_OFFSET_TO_START_SCROLL_FRACTION*estimated_char_bounding_size.height ()),
						int (MIN_OFFSET_TO_START_SCROLL_MIN_PIXELS));
	    estimated_scroll_step_offset = qMax (int (SCROLL_STEP_FRACTION*estimated_char_bounding_size.height ()),
						 int (SCROLL_STEP_MIN_PIXELS));
	    estimated_minute_add_rect = QRect (current_rect.width ()/2 - estimated_char_bounding_size.width (),
					       current_rect.height () - estimated_char_bounding_size.height () -
					       estimated_arrow_bounding_size.width ()*2,
					       estimated_char_bounding_size.width ()*2,
					       estimated_arrow_bounding_size.width ()*2);
	    estimated_minute_subtract_rect = QRect (current_rect.width ()/2 - estimated_char_bounding_size.width (),
						    current_rect.height () - estimated_arrow_bounding_size.width ()*2,
						    estimated_char_bounding_size.width ()*2,
						    estimated_arrow_bounding_size.width ()*2);
	    estimated_minute_subtract_rect = QRect (current_rect.width ()/2 - estimated_char_bounding_size.width (),
						    current_rect.height () - estimated_arrow_bounding_size.width ()*2,
						    estimated_char_bounding_size.width ()*2,
						    estimated_arrow_bounding_size.width ()*2);
	}
    } break;
    case 1: {
	QPainter p (&estimation_picture);
	p.setFont (font);
	const QRect &current_rect = rect ();
	QSize current_size = current_rect.size ();
	if (current_size != estimated_size) {
	    int w = current_size.width ();
	    int h = current_size.height ();
	    int half_w = w/2;
	    QSize char_bounding_size = charBoundingSize (p);
	    QSize arrow_bounding_size = arrowBoundingSize (p);
	    QSize bounding_size = QSize (char_bounding_size.width ()*8, char_bounding_size.height ()*2 + arrow_bounding_size.width ()*2);
	    if ((bounding_size.width () <= w) && (bounding_size.height () <= h)) {
		// Increasing size
		while (estimated_font_size < MAX_FONT_SIZE) {
		    font.setPixelSize (estimated_font_size + 1);
		    p.setFont (font);
		    char_bounding_size = charBoundingSize (p);
		    arrow_bounding_size = arrowBoundingSize (p);
		    bounding_size = QSize (char_bounding_size.width ()*8, char_bounding_size.height ()*2 + arrow_bounding_size.width ()*2);
		    if ((bounding_size.width () > current_size.width ()) ||
			(bounding_size.height () > current_size.height ()))
			break;
		    ++estimated_font_size;
		}
	    } else {
		// Decreasing size
		while (estimated_font_size > MIN_FONT_SIZE) {
		    font.setPixelSize (estimated_font_size);
		    p.setFont (font);
		    char_bounding_size = charBoundingSize (p);
		    arrow_bounding_size = arrowBoundingSize (p);
		    bounding_size = QSize (char_bounding_size.width ()*8, char_bounding_size.height ()*2 + arrow_bounding_size.width ()*2);
		    if ((bounding_size.width () <= w) && (bounding_size.height () <= h))
			break;
		    --estimated_font_size;
		}
	    }
	    font.setPixelSize (estimated_font_size);
	    setFont (font);
	    estimated_size = current_size;
	    estimated_char_bounding_size = charBoundingSize (p);
	    estimated_arrow_bounding_size = arrowBoundingSize (p);
	    estimated_separator_bounding_size = separatorBoundingSize (p);
	    estimated_min_scroll_offset = qMax (int (MIN_OFFSET_TO_START_SCROLL_FRACTION*estimated_char_bounding_size.height ()),
						int (MIN_OFFSET_TO_START_SCROLL_MIN_PIXELS));
	    estimated_scroll_step_offset = qMax (int (SCROLL_STEP_FRACTION*estimated_char_bounding_size.height ()),
						 int (SCROLL_STEP_MIN_PIXELS));
	    estimated_minute_add_rect = QRect (half_w - estimated_char_bounding_size.width (),
					       current_rect.height () - estimated_char_bounding_size.height () -
					       estimated_arrow_bounding_size.width ()*2,
					       estimated_char_bounding_size.width ()*2,
					       estimated_arrow_bounding_size.width ()*2);
	    estimated_minute_subtract_rect = QRect (half_w - estimated_char_bounding_size.width (),
						    current_rect.height () - estimated_arrow_bounding_size.width ()*2,
						    estimated_char_bounding_size.width ()*2,
						    estimated_arrow_bounding_size.width ()*2);
	    int base_y_start = current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ();
	    estimated_minute_scroll_rect = QRect (half_w - estimated_char_bounding_size.width ()*3 - estimated_separator_bounding_size.width ()*2,
					   base_y_start,
					   estimated_char_bounding_size.width ()*6 + estimated_separator_bounding_size.width ()*4,
					   estimated_char_bounding_size.height ());
	}
    } break;
    case 2: {
	QPainter p (&estimation_picture);
	p.setFont (font);
	QRect fake_rect (0, 0, 0, 0);
	QRect current_rect = rect ();
	QSize current_size = current_rect.size ();
	if (current_size != estimated_size) {
	    int w = current_size.width ();
	    int h = current_size.height ();
	    QSize char_bounding_size = charBoundingSize (p);
	    QSize arrow_bounding_size = arrowBoundingSize (p);
	    QSize bounding_size = QSize (char_bounding_size.width ()*8, char_bounding_size.height ()*2 + arrow_bounding_size.width ()*2);
	    if ((bounding_size.width () <= w) && (bounding_size.height () <= h)) {
		// Increasing size
		while (estimated_font_size < MAX_FONT_SIZE) {
		    font.setPixelSize (estimated_font_size + 1);
		    p.setFont (font);
		    char_bounding_size = charBoundingSize (p);
		    arrow_bounding_size = arrowBoundingSize (p);
		    bounding_size = QSize (char_bounding_size.width ()*8, char_bounding_size.height ()*2 + arrow_bounding_size.width ()*2);
		    if ((bounding_size.width () > current_size.width ()) ||
			(bounding_size.height () > current_size.height ()))
			break;
		    ++estimated_font_size;
		}
	    } else {
		// Decreasing size
		while (estimated_font_size > MIN_FONT_SIZE) {
		    font.setPixelSize (estimated_font_size);
		    p.setFont (font);
		    char_bounding_size = charBoundingSize (p);
		    arrow_bounding_size = arrowBoundingSize (p);
		    bounding_size = QSize (char_bounding_size.width ()*8, char_bounding_size.height ()*2 + arrow_bounding_size.width ()*2);
		    if ((bounding_size.width () <= w) && (bounding_size.height () <= h))
			break;
		    --estimated_font_size;
		}
	    }
	    font.setPixelSize (estimated_font_size);
	    setFont (font);
	    estimated_size = current_size;
	    estimated_char_bounding_size = charBoundingSize (p);
	    estimated_arrow_bounding_size = arrowBoundingSize (p);
	    estimated_separator_bounding_size = separatorBoundingSize (p);
	    estimated_min_scroll_offset = qMax (int (MIN_OFFSET_TO_START_SCROLL_FRACTION*estimated_char_bounding_size.height ()),
						int (MIN_OFFSET_TO_START_SCROLL_MIN_PIXELS));
	    estimated_scroll_step_offset = qMax (int (SCROLL_STEP_FRACTION*estimated_char_bounding_size.height ()),
						 int (SCROLL_STEP_MIN_PIXELS));
	    estimated_minute_add_rect = QRect (current_rect.width ()/2 - estimated_char_bounding_size.width (),
					       current_rect.height () - estimated_char_bounding_size.height () -
					       estimated_arrow_bounding_size.width ()*2,
					       estimated_char_bounding_size.width ()*2,
					       estimated_arrow_bounding_size.width ()*2);
	    estimated_minute_subtract_rect = QRect (current_rect.width ()/2 - estimated_char_bounding_size.width (),
						    current_rect.height () - estimated_arrow_bounding_size.width ()*2,
						    estimated_char_bounding_size.width ()*2,
						    estimated_arrow_bounding_size.width ()*2);
	}
    } break;
    case 3: {
	QPainter p (&estimation_picture);
	p.setFont (font);
	QRect fake_rect (0, 0, 0, 0);
	QRect current_rect = rect ();
	QSize current_size = current_rect.size ();
	if (current_size != estimated_size) {
	    int w = current_size.width ();
	    int h = current_size.height ();
	    QSize char_bounding_size = charBoundingSize (p);
	    QSize arrow_bounding_size = arrowBoundingSize (p);
	    QSize bounding_size = QSize (char_bounding_size.width ()*8, char_bounding_size.height ()*2 + arrow_bounding_size.width ()*2);
	    if ((bounding_size.width () <= w) && (bounding_size.height () <= h)) {
		// Increasing size
		while (estimated_font_size < MAX_FONT_SIZE) {
		    font.setPixelSize (estimated_font_size + 1);
		    p.setFont (font);
		    char_bounding_size = charBoundingSize (p);
		    arrow_bounding_size = arrowBoundingSize (p);
		    bounding_size = QSize (char_bounding_size.width ()*8, char_bounding_size.height ()*2 + arrow_bounding_size.width ()*2);
		    if ((bounding_size.width () > current_size.width ()) ||
			(bounding_size.height () > current_size.height ()))
			break;
		    ++estimated_font_size;
		}
	    } else {
		// Decreasing size
		while (estimated_font_size > MIN_FONT_SIZE) {
		    font.setPixelSize (estimated_font_size);
		    p.setFont (font);
		    char_bounding_size = charBoundingSize (p);
		    arrow_bounding_size = arrowBoundingSize (p);
		    bounding_size = QSize (char_bounding_size.width ()*8, char_bounding_size.height ()*2 + arrow_bounding_size.width ()*2);
		    if ((bounding_size.width () <= w) && (bounding_size.height () <= h))
			break;
		    --estimated_font_size;
		}
	    }
	    font.setPixelSize (estimated_font_size);
	    setFont (font);
	    estimated_size = current_size;
	    estimated_char_bounding_size = charBoundingSize (p);
	    estimated_arrow_bounding_size = arrowBoundingSize (p);
	    estimated_separator_bounding_size = separatorBoundingSize (p);
	    estimated_min_scroll_offset = qMax (int (MIN_OFFSET_TO_START_SCROLL_FRACTION*estimated_char_bounding_size.height ()),
						int (MIN_OFFSET_TO_START_SCROLL_MIN_PIXELS));
	    estimated_scroll_step_offset = qMax (int (SCROLL_STEP_FRACTION*estimated_char_bounding_size.height ()),
						 int (SCROLL_STEP_MIN_PIXELS));
	    estimated_minute_add_rect = QRect (current_rect.width ()/2 - estimated_char_bounding_size.width (),
					       current_rect.height () - estimated_char_bounding_size.height () -
					       estimated_arrow_bounding_size.width ()*2,
					       estimated_char_bounding_size.width ()*2,
					       estimated_arrow_bounding_size.width ()*2);
	    estimated_minute_subtract_rect = QRect (current_rect.width ()/2 - estimated_char_bounding_size.width (),
						    current_rect.height () - estimated_arrow_bounding_size.width ()*2,
						    estimated_char_bounding_size.width ()*2,
						    estimated_arrow_bounding_size.width ()*2);
	}
    } break;
    case 4: {
	QPainter p (&estimation_picture);
	p.setFont (font);
	QRect fake_rect (0, 0, 0, 0);
	QRect current_rect = rect ();
	QSize current_size = current_rect.size ();
	if (current_size != estimated_size) {
	    int w = current_size.width ();
	    int h = current_size.height ();
	    QSize char_bounding_size = charBoundingSize (p);
	    QSize arrow_bounding_size = arrowBoundingSize (p);
	    QSize bounding_size = QSize (char_bounding_size.width ()*8, char_bounding_size.height ()*2 + arrow_bounding_size.width ()*2);
	    if ((bounding_size.width () <= w) && (bounding_size.height () <= h)) {
		// Increasing size
		while (estimated_font_size < MAX_FONT_SIZE) {
		    font.setPixelSize (estimated_font_size + 1);
		    p.setFont (font);
		    char_bounding_size = charBoundingSize (p);
		    arrow_bounding_size = arrowBoundingSize (p);
		    bounding_size = QSize (char_bounding_size.width ()*8, char_bounding_size.height ()*2 + arrow_bounding_size.width ()*2);
		    if ((bounding_size.width () > current_size.width ()) ||
			(bounding_size.height () > current_size.height ()))
			break;
		    ++estimated_font_size;
		}
	    } else {
		// Decreasing size
		while (estimated_font_size > MIN_FONT_SIZE) {
		    font.setPixelSize (estimated_font_size);
		    p.setFont (font);
		    char_bounding_size = charBoundingSize (p);
		    arrow_bounding_size = arrowBoundingSize (p);
		    bounding_size = QSize (char_bounding_size.width ()*8, char_bounding_size.height ()*2 + arrow_bounding_size.width ()*2);
		    if ((bounding_size.width () <= w) && (bounding_size.height () <= h))
			break;
		    --estimated_font_size;
		}
	    }
	    font.setPixelSize (estimated_font_size);
	    setFont (font);
	    estimated_size = current_size;
	    estimated_char_bounding_size = charBoundingSize (p);
	    estimated_arrow_bounding_size = arrowBoundingSize (p);
	    estimated_separator_bounding_size = separatorBoundingSize (p);
	    estimated_min_scroll_offset = qMax (int (MIN_OFFSET_TO_START_SCROLL_FRACTION*estimated_char_bounding_size.height ()),
						int (MIN_OFFSET_TO_START_SCROLL_MIN_PIXELS));
	    estimated_scroll_step_offset = qMax (int (SCROLL_STEP_FRACTION*estimated_char_bounding_size.height ()),
						 int (SCROLL_STEP_MIN_PIXELS));
	    estimated_minute_add_rect = QRect (current_rect.width ()/2 - estimated_char_bounding_size.width (),
					       current_rect.height () - estimated_char_bounding_size.height () -
					       estimated_arrow_bounding_size.width ()*2,
					       estimated_char_bounding_size.width ()*2,
					       estimated_arrow_bounding_size.width ()*2);
	    estimated_minute_subtract_rect = QRect (current_rect.width ()/2 - estimated_char_bounding_size.width (),
						    current_rect.height () - estimated_arrow_bounding_size.width ()*2,
						    estimated_char_bounding_size.width ()*2,
						    estimated_arrow_bounding_size.width ()*2);
	}
    } break;
    }
}
void DigitalTimer::mousePressEvent (QMouseEvent *event)
{
    emit userIsAlive ();
    if (event->button () == Qt::LeftButton) {
	down = true;
	emit lmb_pressed ();
	const QPoint &pos = event->pos ();
	if (edit_mode.isEnabled ()) {
	    if (!edit_mode.isBlocked ()) {
		if (estimated_minute_add_rect.contains (pos)) {
		    button_pressed = AddMinutePressed;
		    button_pressed_rect = estimated_minute_add_rect;
		} else if (estimated_minute_subtract_rect.contains (pos)) {
		    button_pressed = SubtractMinutePressed;
		    button_pressed_rect = estimated_minute_subtract_rect;
		} else if (estimated_minute_scroll_rect.contains (pos)) {
		    button_pressed = ScrollMinutePressed;
		    button_pressed_rect = rect ();
		    vertical_scroll_accum = 0;
		} else {
		    button_pressed = LeaveAreaPressed;
		    button_pressed_rect = estimated_minute_scroll_rect;
		}
	    }
	    update ();
	} else {
	    emit enterEditModeRequested ();
	}
	button_pressed_point = pos;
	previous_point = pos;
    }
}
void DigitalTimer::mouseReleaseEvent (QMouseEvent *event)
{
    previous_point = QPoint (-1000000, -1000000);
    emit userIsAlive ();
    if (event->button () == Qt::LeftButton) {
	down = false;
	emit lmb_released ();
	const QPoint &pos = event->pos ();
	if (button_pressed == LeaveAreaPressed) {
	    if (!estimated_minute_scroll_rect.contains (pos))
		emit leaveEditModeRequested ();
	} else if ((button_pressed != NonePressed) && button_pressed_rect.contains (pos)) {
	    switch (button_pressed) {
	    case AddMinutePressed: {
		addMinuteSharp ();
	    } break;
	    case SubtractMinutePressed: {
		subtractMinuteSharp ();
	    } break;
	    default: {
	    } break;
	    }
	}
	button_pressed = NonePressed;
	update ();
    }
}
void DigitalTimer::mouseMoveEvent (QMouseEvent *event)
{
    emit userIsAlive ();
    const QPoint &pos = event->pos ();
    switch (button_pressed) {
    case LeaveAreaPressed:
    case AddMinutePressed:
    case SubtractMinutePressed:	{
	int dy = pos.y () - button_pressed_point.y ();
	if (qAbs (dy) < estimated_min_scroll_offset)
	    break;
	button_pressed = ScrollMinutePressed;
	vertical_scroll_accum = (dy > 0) ? (dy - estimated_min_scroll_offset) : (dy + estimated_min_scroll_offset);
    }
    case ScrollMinutePressed: {
	if (previous_point.y () > -1000000)
	    vertical_scroll_accum += previous_point.y () - pos.y ();
	if (vertical_scroll_accum > 0) {
	    while (vertical_scroll_accum >= estimated_scroll_step_offset) {
		addMinuteSharp ();
		vertical_scroll_accum -= estimated_scroll_step_offset;
	    }
	}
	if (vertical_scroll_accum < 0) {
	    while (vertical_scroll_accum <= -estimated_scroll_step_offset) {
		subtractMinuteSharp ();
		vertical_scroll_accum += estimated_scroll_step_offset;
	    }
	}
	update ();
    } break;
    default: {
	button_pressed_point = pos;
	previous_point = pos;
    }
    }
    previous_point = pos;
}
QSize DigitalTimer::charBoundingSize (QPainter &p)
{
    QRect fake_rect (0, 0, 0, 0);
    QSize max_size = p.boundingRect (fake_rect, TEXT_FLAGS, "0").size ();
    for (int digit = 1; digit < 10; ++digit)
	max_size = max_size.expandedTo (p.boundingRect (fake_rect, TEXT_FLAGS, QString::number (digit)).size ());
    return max_size;
}
QSize DigitalTimer::arrowBoundingSize (QPainter &p)
{
    return p.boundingRect (QRect (0, 0, 0, 0), TEXT_FLAGS, ">").size ();
}
QSize DigitalTimer::separatorBoundingSize (QPainter &p)
{
    return p.boundingRect (QRect (0, 0, 0, 0), TEXT_FLAGS, ":").size ();
}
void DigitalTimer::paintEvent (QPaintEvent*)
{
    Timer *timer = app_manager->getCurrentTimer ();
    QTime value = timer->getTimeLeft ();
    QPainter p (this);
    if (KITCHENTIMER_SHOW_DEBUG_OVERLAY) {
	p.save ();
	p.setPen (Qt::NoPen);
	p.setBrush (QColor (0, 0x88, 0, 60));
	p.drawRect (rect ());
	p.restore ();
    }
    switch (KITCHENTIMER_DIGITAL_TIMER_MODE) {
    case 0: {
	QString hour_text = value.toString ("hh");
	QString minute_text = value.toString ("mm");
	QString second_text = value.toString ("ss");
	QColor text_color (QColor (0xff, 0xff, 0xfd));
	QColor shadow_color (0xbb, 0x9b, 0x79);
	p.setPen (text_color);

	const QRect &current_rect = rect ();
 	int base_y_start = current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ();
	{
	    QPoint base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*6)/2
					- estimated_separator_bounding_size.width ()*2,
					base_y_start);
	    for (int i = 0; i < 2; ++i) {
		p.setPen (shadow_color);
		p.setCompositionMode (QPainter::CompositionMode_Multiply);
		p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    hour_text.mid (i, 1));
		p.setPen (text_color);
		p.setCompositionMode (QPainter::CompositionMode_SourceOver);
		p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    hour_text.mid (i, 1));
		base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
	    }
	}
	{
	    QPoint base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*4)/2
					+ estimated_separator_bounding_size.width ()/2,
					base_y_start);
	    p.setPen (shadow_color);
	    p.setCompositionMode (QPainter::CompositionMode_Multiply);
	    p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	    p.setPen (text_color);
	    p.setCompositionMode (QPainter::CompositionMode_SourceOver);
	    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	}
	{
	    QPoint base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*2)/2,
					base_y_start);
	    for (int i = 0; i < 2; ++i) {
		p.setPen (shadow_color);
		p.setCompositionMode (QPainter::CompositionMode_Multiply);
		p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    minute_text.mid (i, 1));
		p.setPen (text_color);
		p.setCompositionMode (QPainter::CompositionMode_SourceOver);
		p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    minute_text.mid (i, 1));
		base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
	    }
	}
	{
	    QPoint base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*2)/2
					- estimated_separator_bounding_size.width ()/2,
					base_y_start);
	    p.setPen (shadow_color);
	    p.setCompositionMode (QPainter::CompositionMode_Multiply);
	    p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	    p.setPen (text_color);
	    p.setCompositionMode (QPainter::CompositionMode_SourceOver);
	    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	}
	{
	    QPoint base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*2)/2
					+ estimated_separator_bounding_size.width ()*2,
					base_y_start);
	    for (int i = 0; i < 2; ++i) {
		p.setPen (shadow_color);
		p.setCompositionMode (QPainter::CompositionMode_Multiply);
		p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    second_text.mid (i, 1));
		p.setPen (text_color);
		p.setCompositionMode (QPainter::CompositionMode_SourceOver);
		p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    second_text.mid (i, 1));
		base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
	    }
	}
	if (timer->isRunning ()) { // Period
	    double font_scale = 0.9;
	    p.save ();
	    font2.setPixelSize (estimated_font_size*font_scale);
	    p.setFont (font2);
	    int base_y_start = current_rect.height () - estimated_char_bounding_size.height ()*1.8 - estimated_arrow_bounding_size.width ();
	    QTime value = timer->getPeriod ();
	    QString hour_text = value.toString ("hh");
	    QString minute_text = value.toString ("mm");
	    QString second_text = value.toString ("ss");
	    QColor shadow_color (0xbb, 0x9b, 0x79, 0xee);
	    p.setPen (shadow_color);
	    {
		QPoint base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*font_scale*6)/2
					    - estimated_separator_bounding_size.width ()*font_scale*2,
					    base_y_start);
		for (int i = 0; i < 2; ++i) {
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size*font_scale), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				hour_text.mid (i, 1));
		    base_point.setX (base_point.x () + estimated_char_bounding_size.width ()*font_scale);
		}
	    }
	    {
		QPoint base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*font_scale*4)/2
					    + estimated_separator_bounding_size.width ()*font_scale/2,
					    base_y_start);
		p.setCompositionMode (QPainter::CompositionMode_Multiply);
		p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size*font_scale), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	    }
	    {
		QPoint base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*font_scale*2)/2,
					    base_y_start);
		for (int i = 0; i < 2; ++i) {
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size*font_scale), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				minute_text.mid (i, 1));
		    base_point.setX (base_point.x () + estimated_char_bounding_size.width ()*font_scale);
		}
	    }
	    {
		QPoint base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*font_scale*2)/2
					    - estimated_separator_bounding_size.width ()*font_scale/2,
					    base_y_start);
		p.setCompositionMode (QPainter::CompositionMode_Multiply);
		p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size*font_scale), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	    }
	    {
		QPoint base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*font_scale*2)/2
					    + estimated_separator_bounding_size.width ()*font_scale*2,
					    base_y_start);
		for (int i = 0; i < 2; ++i) {
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size*font_scale), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				second_text.mid (i, 1));
		    base_point.setX (base_point.x () + estimated_char_bounding_size.width ()*font_scale);
		}
	    }
	    p.restore ();
	}
	if (!edit_mode.isEnabled ()) { // "hour min sec"
	    p.save ();
	    font2.setPixelSize (estimated_font_size*0.5);
	    p.setFont (font2);

	    p.setCompositionMode (QPainter::CompositionMode_Multiply);
	    p.setPen (QColor (0xbb, 0x9b, 0x79));

	    p.drawText (QRect (current_rect.x () - estimated_char_bounding_size.width ()*2 - estimated_separator_bounding_size.width ()*2,
			       current_rect.y (),
			       current_rect.width (),
			       current_rect.height ()),
			Qt::AlignBottom | Qt::AlignHCenter | Qt::TextDontClip, "час");

	    p.drawText (QRect (current_rect.x (), current_rect.y (),
			       current_rect.width (), current_rect.height ()),
			Qt::AlignBottom | Qt::AlignHCenter | Qt::TextDontClip, "мин");

	    p.drawText (QRect (current_rect.x () + estimated_char_bounding_size.width ()*2 + estimated_separator_bounding_size.width ()*2,
			       current_rect.y (),
			       current_rect.width (),
			       current_rect.height ()),
			Qt::AlignBottom | Qt::AlignHCenter | Qt::TextDontClip, "сек");

	    p.restore ();
	}
	if (edit_mode.isEnabled ()) {
	    QPoint base_point = QPoint (current_rect.width ()/2,
					current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	    QColor basic_color (0xd5, 0x92, 0x4b);
	    QColor pressed_color (0x16, 0x83, 0x87);
	    {
		QTransform tr;
		tr.translate (base_point.x (), base_point.y ());
		tr.scale (0.75, 0.75);
		tr.translate (-estimated_arrow_bounding_size.height ()/2, 0);
		tr.rotate (-90);
		tr.translate (-base_point.x (), -base_point.y ());
		p.setTransform (tr);
		p.setPen (((button_pressed == AddMinutePressed) || (button_pressed == ScrollMinutePressed)) ? pressed_color : basic_color);
		p.drawText (QRect (base_point, estimated_arrow_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ">");
	    }
	    {
		QTransform tr;
		tr.translate (base_point.x (), base_point.y ());
		tr.scale (0.75, 0.75);
		tr.translate (-estimated_arrow_bounding_size.height ()/2,
			      estimated_char_bounding_size.height () + estimated_arrow_bounding_size.width ());
		tr.rotate (-90);
		tr.scale (-1.0, 1.0);
		tr.translate (-base_point.x (), -base_point.y ());
		p.setTransform (tr);
		p.setPen (((button_pressed == SubtractMinutePressed) || (button_pressed == ScrollMinutePressed)) ? pressed_color : basic_color);
		p.drawText (QRect (base_point, estimated_arrow_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ">");
	    }
	}
    } break;
    case 1: {
	QString hour_text = value.toString ("hh");
	QString minute_text = value.toString ("mm");
	QString second_text = value.toString ("ss");
	QString next_second_text = QString ().sprintf ("%02d", (value.second () + 1)%60);
	QColor text_color (QColor (0xff, 0xff, 0xfd));
	QColor shadow_color (0xbb, 0x9b, 0x79);
	p.setPen (text_color);

	const QRect &current_rect = rect ();
	int base_y_start = current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ();
	{ // Hours
	    QPoint base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*6)/2
					- estimated_separator_bounding_size.width ()*2,
					base_y_start);
	    for (int i = 0; i < 2; ++i) {
		p.setPen (shadow_color);
		p.setCompositionMode (QPainter::CompositionMode_Multiply);
		p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    hour_text.mid (i, 1));
		p.setPen (text_color);
		p.setCompositionMode (QPainter::CompositionMode_SourceOver);
		p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    hour_text.mid (i, 1));
		base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
	    }
	}
	{ // ":"
	    QPoint base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*4)/2
					+ estimated_separator_bounding_size.width ()/2,
					base_y_start);
	    p.setPen (shadow_color);
	    p.setCompositionMode (QPainter::CompositionMode_Multiply);
	    p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	    p.setPen (text_color);
	    p.setCompositionMode (QPainter::CompositionMode_SourceOver);
	    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	}
	{ // Minutes
	    QPoint base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*2)/2,
					base_y_start);
	    for (int i = 0; i < 2; ++i) {
		p.setPen (shadow_color);
		p.setCompositionMode (QPainter::CompositionMode_Multiply);
		p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    minute_text.mid (i, 1));
		p.setPen (text_color);
		p.setCompositionMode (QPainter::CompositionMode_SourceOver);
		p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    minute_text.mid (i, 1));
		base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
	    }
	}
	{ // ":"
	    QPoint base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*2)/2
					- estimated_separator_bounding_size.width ()/2,
					base_y_start);
	    p.setPen (shadow_color);
	    p.setCompositionMode (QPainter::CompositionMode_Multiply);
	    p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	    p.setPen (text_color);
	    p.setCompositionMode (QPainter::CompositionMode_SourceOver);
	    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	}
	int msec = value.msec ();
	if (timer->isRunning ()) {
	    if (msec < 400) { // Seconds
		double factor = msec/400.0;
		QColor c1 (shadow_color);
		c1.setAlpha (factor*255.0);
		QColor c2 (text_color);
		c2.setAlpha (factor*255.0);
		QPoint base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*2)/2
					    + estimated_separator_bounding_size.width ()*2,
					    base_y_start);
		{
		    QTransform tr;
		    int dx = base_point.x () + estimated_char_bounding_size.width ()/2;
		    int dy = base_point.y () + estimated_char_bounding_size.height ()/2;
		    tr.translate (dx, dy);
		    double scale = factor*0.2 + 0.7;
		    tr.scale (scale, scale);
		    tr.translate (-dx, -dy);
		    p.setWorldTransform (tr);
		    p.setPen (c1);
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				second_text.mid (0, 1));
		    p.setPen (c2);
		    p.setCompositionMode (QPainter::CompositionMode_SourceOver);
		    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				second_text.mid (0, 1));
		    p.resetTransform ();
		    base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
		}
		{
		    QTransform tr;
		    int dx = base_point.x () + estimated_char_bounding_size.width ()/2;
		    int dy = base_point.y () + estimated_char_bounding_size.height ()/2;
		    tr.translate (dx, dy);
		    double scale = factor*0.2 + 0.7;
		    tr.scale (scale, scale);
		    tr.translate (-dx, -dy);
		    p.setWorldTransform (tr);
		    p.setPen (c1);
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				second_text.mid (1, 1));
		    p.setPen (c2);
		    p.setCompositionMode (QPainter::CompositionMode_SourceOver);
		    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				second_text.mid (1, 1));
		    p.resetTransform ();
		}
	    } else { // Next second
		QColor c1 (shadow_color);
		c1.setAlpha (value.msec ()*0.001*255.0);
		QColor c2 (text_color);
		c2.setAlpha (value.msec ()*0.001*255.0);
		QPoint base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*2)/2
					    + estimated_separator_bounding_size.width ()*2,
					    base_y_start);
		QEasingCurve curve (QEasingCurve::InElastic);
		double scale = 1.0 - curve.valueForProgress ((msec - 600)/600.0);
		{
		    QTransform tr;
		    int dx = base_point.x () + estimated_char_bounding_size.width ()/2;
		    int dy = base_point.y () + estimated_char_bounding_size.height ()/2;
		    tr.translate (dx, dy);
		    tr.scale (scale, scale);
		    tr.translate (-dx, -dy);
		    p.setWorldTransform (tr);
		    p.setPen (shadow_color);
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				second_text.mid (0, 1));
		    p.setPen (text_color);
		    p.setCompositionMode (QPainter::CompositionMode_SourceOver);
		    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				second_text.mid (0, 1));
		    p.resetTransform ();
		    base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
		}
		{
		    QTransform tr;
		    int dx = base_point.x () + estimated_char_bounding_size.width ()/2;
		    int dy = base_point.y () + estimated_char_bounding_size.height ()/2;
		    tr.translate (dx, dy);
		    tr.scale (scale, scale);
		    tr.translate (-dx, -dy);
		    p.setWorldTransform (tr);
		    p.setPen (shadow_color);
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				second_text.mid (1, 1));
		    p.setPen (text_color);
		    p.setCompositionMode (QPainter::CompositionMode_SourceOver);
		    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				second_text.mid (1, 1));
		    p.resetTransform ();
		}
	    }
	} else {
	    {
		QPoint base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*2)/2
					    + estimated_separator_bounding_size.width ()*2,
					    base_y_start);
		for (int i = 0; i < 2; ++i) {
		    p.setPen (shadow_color);
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				second_text.mid (i, 1));
		    p.setPen (text_color);
		    p.setCompositionMode (QPainter::CompositionMode_SourceOver);
		    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				second_text.mid (i, 1));
		    base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
		}
	    }
	}
	if (timer->getPeriod ().isValid () && !edit_mode.isEnabled ()) { // Period
	    double font_scale = 0.9;
	    p.save ();
	    font2.setPixelSize (estimated_font_size*font_scale);
	    p.setFont (font2);
	    int base_y_start = current_rect.height () - estimated_char_bounding_size.height ()*1.8 - estimated_arrow_bounding_size.width ();
	    QTime value = timer->getPeriod ();
	    QString hour_text = value.toString ("hh");
	    QString minute_text = value.toString ("mm");
	    QString second_text = value.toString ("ss");
	    QColor shadow_color (0xbb, 0x9b, 0x79, 0xee);
	    p.setPen (shadow_color);
	    {
		QPoint base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*font_scale*6)/2
					    - estimated_separator_bounding_size.width ()*font_scale*2,
					    base_y_start);
		for (int i = 0; i < 2; ++i) {
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size*font_scale), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				hour_text.mid (i, 1));
		    base_point.setX (base_point.x () + estimated_char_bounding_size.width ()*font_scale);
		}
	    }
	    {
		QPoint base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*font_scale*4)/2
					    + estimated_separator_bounding_size.width ()*font_scale/2,
					    base_y_start);
		p.setCompositionMode (QPainter::CompositionMode_Multiply);
		p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size*font_scale), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	    }
	    {
		QPoint base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*font_scale*2)/2,
					    base_y_start);
		for (int i = 0; i < 2; ++i) {
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size*font_scale), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				minute_text.mid (i, 1));
		    base_point.setX (base_point.x () + estimated_char_bounding_size.width ()*font_scale);
		}
	    }
	    {
		QPoint base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*font_scale*2)/2
					    - estimated_separator_bounding_size.width ()*font_scale/2,
					    base_y_start);
		p.setCompositionMode (QPainter::CompositionMode_Multiply);
		p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size*font_scale), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	    }
	    {
		QPoint base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*font_scale*2)/2
					    + estimated_separator_bounding_size.width ()*font_scale*2,
					    base_y_start);
		for (int i = 0; i < 2; ++i) {
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size*font_scale), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				second_text.mid (i, 1));
		    base_point.setX (base_point.x () + estimated_char_bounding_size.width ()*font_scale);
		}
	    }
	    p.restore ();
	}
	if (!edit_mode.isEnabled ()) { // "hour min sec"
	    p.save ();
	    font2.setPixelSize (estimated_font_size*0.5);
	    p.setFont (font2);

	    p.setCompositionMode (QPainter::CompositionMode_Multiply);
	    p.setPen (QColor (0xbb, 0x9b, 0x79));

	    p.drawText (QRect (current_rect.x () - estimated_char_bounding_size.width ()*2 - estimated_separator_bounding_size.width ()*2,
			       current_rect.y (),
			       current_rect.width (),
			       current_rect.height ()),
			Qt::AlignBottom | Qt::AlignHCenter | Qt::TextDontClip, "час");

	    p.drawText (QRect (current_rect.x (), current_rect.y (),
			       current_rect.width (), current_rect.height ()),
			Qt::AlignBottom | Qt::AlignHCenter | Qt::TextDontClip, "мин");

	    p.drawText (QRect (current_rect.x () + estimated_char_bounding_size.width ()*2 + estimated_separator_bounding_size.width ()*2,
			       current_rect.y (),
			       current_rect.width (),
			       current_rect.height ()),
			Qt::AlignBottom | Qt::AlignHCenter | Qt::TextDontClip, "сек");

	    p.restore ();
	}
	if (edit_mode.isEnabled () && !edit_mode.isBlocked ()) {
	    QPoint base_point = QPoint (current_rect.width ()/2,
					current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	    QColor basic_color (0xd5, 0x92, 0x4b);
	    QColor pressed_color (0x16, 0x83, 0x87);
	    {
		QTransform tr;
		tr.translate (base_point.x (), base_point.y ());
		tr.scale (0.75, 0.75);
		tr.translate (-estimated_arrow_bounding_size.height ()/2, 0);
		tr.rotate (-90);
		tr.translate (-base_point.x (), -base_point.y ());
		p.setTransform (tr);
		p.setPen ((!edit_mode.isHalted () && ((button_pressed == AddMinutePressed) || (button_pressed == ScrollMinutePressed))) ?
			  pressed_color : basic_color);
		p.drawText (QRect (base_point, estimated_arrow_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ">");
	    }
	    {
		QTransform tr;
		tr.translate (base_point.x (), base_point.y ());
		tr.scale (0.75, 0.75);
		tr.translate (-estimated_arrow_bounding_size.height ()/2,
			      estimated_char_bounding_size.height () + estimated_arrow_bounding_size.width ());
		tr.rotate (-90);
		tr.scale (-1.0, 1.0);
		tr.translate (-base_point.x (), -base_point.y ());
		p.setTransform (tr);
		p.setPen ((!edit_mode.isHalted () && ((button_pressed == SubtractMinutePressed) || (button_pressed == ScrollMinutePressed))) ?
			  pressed_color : basic_color);
		p.drawText (QRect (base_point, estimated_arrow_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ">");
	    }
	}
    } break;
    case 2: {
	QString hour_text = value.toString ("hh");
	QString minute_text = value.toString ("mm");
	QString second_text = value.toString ("ss");
	QColor text_color (QColor (0xff, 0xff, 0xfd));
	QColor shadow_color (0xbb, 0x9b, 0x79);
	p.setPen (text_color);

	const QRect &current_rect = rect ();
	QPoint base_point ((current_rect.width () - estimated_char_bounding_size.width ()*8)/2,
			   current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	{
	    base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*6)/2 - estimated_separator_bounding_size.width ()*2,
				 current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	    for (int i = 0; i < 2; ++i) {
		p.setPen (shadow_color);
		p.setCompositionMode (QPainter::CompositionMode_Multiply);
		p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    hour_text.mid (i, 1));

		p.setPen (text_color);
		p.setCompositionMode (QPainter::CompositionMode_SourceOver);
		p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    hour_text.mid (i, 1));

		base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
	    }
	}
	{
	    base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*4)/2 + estimated_separator_bounding_size.width ()/2,
				 current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());

	    p.setPen (shadow_color);
	    p.setCompositionMode (QPainter::CompositionMode_Multiply);
	    p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");

	    p.setPen (text_color);
	    p.setCompositionMode (QPainter::CompositionMode_SourceOver);
	    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	}
	{
	    base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*2)/2,
				 current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	    for (int i = 0; i < 2; ++i) {
		p.setPen (shadow_color);
		p.setCompositionMode (QPainter::CompositionMode_Multiply);
		p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    minute_text.mid (i, 1));

		p.setPen (text_color);
		p.setCompositionMode (QPainter::CompositionMode_SourceOver);
		p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    minute_text.mid (i, 1));

		base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
	    }
	}
	{
	    base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*2)/2 - estimated_separator_bounding_size.width ()/2,
				 current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());

	    p.setPen (shadow_color);
	    p.setCompositionMode (QPainter::CompositionMode_Multiply);
	    p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");

	    p.setPen (text_color);
	    p.setCompositionMode (QPainter::CompositionMode_SourceOver);
	    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	}
	{
	    base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*2)/2 + estimated_separator_bounding_size.width ()*2,
				 current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	    for (int i = 0; i < 2; ++i) {
		p.setPen (shadow_color);
		p.setCompositionMode (QPainter::CompositionMode_Multiply);
		p.drawText (QRect (base_point + QPoint (2, 3), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    second_text.mid (i, 1));

		p.setPen (text_color);
		p.setCompositionMode (QPainter::CompositionMode_SourceOver);
		p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    second_text.mid (i, 1));

		base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
	    }
	}
	if (!edit_mode.isEnabled ()) {
	    p.save ();
	    font2.setPixelSize (estimated_font_size*0.5);
	    p.setFont (font2);

	    p.setCompositionMode (QPainter::CompositionMode_Multiply);
	    p.setPen (QColor (0xbb, 0x9b, 0x79));

	    p.drawText (QRect (current_rect.x () - estimated_char_bounding_size.width ()*2 - estimated_separator_bounding_size.width ()*2,
			       current_rect.y (),
			       current_rect.width (),
			       current_rect.height ()),
			Qt::AlignBottom | Qt::AlignHCenter | Qt::TextDontClip, "час");

	    p.drawText (QRect (current_rect.x (), current_rect.y (),
			       current_rect.width (), current_rect.height ()),
			Qt::AlignBottom | Qt::AlignHCenter | Qt::TextDontClip, "мин");

	    p.drawText (QRect (current_rect.x () + estimated_char_bounding_size.width ()*2 + estimated_separator_bounding_size.width ()*2,
			       current_rect.y (),
			       current_rect.width (),
			       current_rect.height ()),
			Qt::AlignBottom | Qt::AlignHCenter | Qt::TextDontClip, "сек");

	    p.restore ();
	}
	base_point = QPoint (current_rect.width ()/2,
			     current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	if (edit_mode.isEnabled ()) {
	    QColor basic_color (0xd5, 0x92, 0x4b);
	    QColor pressed_color (0x16, 0x83, 0x87);
	    {
		QTransform tr;
		tr.translate (base_point.x (), base_point.y ());
		tr.scale (0.75, 0.75);
		tr.translate (-estimated_arrow_bounding_size.height ()/2, 0);
		tr.rotate (-90);
		tr.translate (-base_point.x (), -base_point.y ());
		p.setTransform (tr);
		p.setPen (((button_pressed == AddMinutePressed) || (button_pressed == ScrollMinutePressed)) ? pressed_color : basic_color);
		p.drawText (QRect (base_point, estimated_arrow_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ">");
	    }
	    {
		QTransform tr;
		tr.translate (base_point.x (), base_point.y ());
		tr.scale (0.75, 0.75);
		tr.translate (-estimated_arrow_bounding_size.height ()/2,
			      estimated_char_bounding_size.height () + estimated_arrow_bounding_size.width ());
		tr.rotate (-90);
		tr.scale (-1.0, 1.0);
		tr.translate (-base_point.x (), -base_point.y ());
		p.setTransform (tr);
		p.setPen (((button_pressed == SubtractMinutePressed) || (button_pressed == ScrollMinutePressed)) ? pressed_color : basic_color);
		p.drawText (QRect (base_point, estimated_arrow_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ">");
	    }
	}
    } break;
    case 3: {
	QString hour_text = value.toString ("h");
	QString minute_text = value.toString ("mm");
	QString second_text = value.toString ("ss");
	p.setPen (QColor (0xff, 0xff, 0xfd));

	const QRect &current_rect = rect ();
	QPoint base_point ((current_rect.width () - estimated_char_bounding_size.width ()*8)/2,
			   current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	{
	    base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*4)/2 - estimated_separator_bounding_size.width ()*2,
				 current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	    {
		p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    hour_text.right (1));
		base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
	    }
	}
	{
	    base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*4)/2 + estimated_separator_bounding_size.width ()/2,
				 current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	}
	{
	    base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*2)/2,
				 current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	    for (int i = 0; i < 2; ++i) {
		p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    minute_text.mid (i, 1));
		base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
	    }
	}
	{
	    p.save ();
	    font2.setPixelSize (estimated_font_size*0.75);
	    p.setFont (font2);
	    base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*2)/2 + estimated_separator_bounding_size.width (),
				 current_rect.height () - estimated_char_bounding_size.height ()*(0.75 + 0.125 + 0.125*0.3) - estimated_arrow_bounding_size.width ());
	    for (int i = 0; i < 2; ++i) {
		p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    second_text.mid (i, 1));
		base_point.setX (base_point.x () + estimated_char_bounding_size.width ()*0.75);
	    }
	    p.restore ();
	}
	base_point = QPoint (current_rect.width ()/2,
			     current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	if (edit_mode.isEnabled ()) {
	    QColor basic_color (0xd5, 0x92, 0x4b);
	    QColor pressed_color (0x16, 0x83, 0x87);
	    {
		QTransform tr;
		tr.translate (base_point.x (), base_point.y ());
		tr.scale (0.75, 0.75);
		tr.translate (-estimated_arrow_bounding_size.height ()/2, 0);
		tr.rotate (-90);
		tr.translate (-base_point.x (), -base_point.y ());
		p.setTransform (tr);
		p.setPen (((button_pressed == AddMinutePressed) || (button_pressed == ScrollMinutePressed)) ? pressed_color : basic_color);
		p.drawText (QRect (base_point, estimated_arrow_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ">");
	    }
	    {
		QTransform tr;
		tr.translate (base_point.x (), base_point.y ());
		tr.scale (0.75, 0.75);
		tr.translate (-estimated_arrow_bounding_size.height ()/2,
			      estimated_char_bounding_size.height () + estimated_arrow_bounding_size.width ());
		tr.rotate (-90);
		tr.scale (-1.0, 1.0);
		tr.translate (-base_point.x (), -base_point.y ());
		p.setTransform (tr);
		p.setPen (((button_pressed == SubtractMinutePressed) || (button_pressed == ScrollMinutePressed)) ? pressed_color : basic_color);
		p.drawText (QRect (base_point, estimated_arrow_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ">");
	    }
	}
    } break;
    case 4: {
	int sec_phase = (QTime (0, 0, 0).msecsTo (value)/100)%10;
	QString hour_text = value.toString ("h");
	QString minute_text = value.toString ("mm");
	QString second_text = value.toString ("ss");
	p.setPen (QColor (0xff, 0xff, 0xfd));

	const QRect &current_rect = rect ();
	QPoint base_point ((current_rect.width () - estimated_char_bounding_size.width ()*8)/2,
			   current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	{
	    base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*4)/2 - estimated_separator_bounding_size.width ()*2,
				 current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	    {
		p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    hour_text.right (1));
		base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
	    }
	}
	{
	    base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*4)/2 + estimated_separator_bounding_size.width ()/2,
				 current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	}
	{
	    base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*2)/2,
				 current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	    for (int i = 0; i < 2; ++i) {
		p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    minute_text.mid (i, 1));
		base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
	    }
	}
	if (!edit_mode.isEnabled ()) { // Tough animation should be here
	    p.save ();
	    p.setPen (QColor (0xff, 0xff, 0xfd));
	    font2.setPixelSize (estimated_font_size*0.75);
	    p.setFont (font2);
	    QString second_text2 = QString ().sprintf ("%02d", (value.second () + 1)%60);

	    QSize im_size (estimated_char_bounding_size.width ()*2 + 2,
			   estimated_char_bounding_size.height () + 2);
	    QImage current_text_im (im_size, QImage::Format_ARGB32);
	    QImage next_text_im (im_size, QImage::Format_ARGB32);
	    QImage mask_im (im_size, QImage::Format_ARGB32);
	    current_text_im.fill (0);
	    next_text_im.fill (0);
	    mask_im.fill (0);
	    
	    QPainter p2;

	    p2.begin (&current_text_im);
	    p2.setRenderHint (QPainter::Antialiasing, true);
	    p2.setFont (font2);
	    base_point = QPoint (0, 0);
	    for (int i = 0; i < 2; ++i) {
		p2.setPen (QColor (0, 0, 0, 0xcc));
		p2.drawText (QRect (base_point + QPoint (2, 2), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			     second_text.mid (i, 1));
		p2.setPen (QColor (0xff, 0xff, 0xfd));
		p2.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			     second_text.mid (i, 1));
		base_point.setX (base_point.x () + estimated_char_bounding_size.width ()*0.75);
	    }
	    p2.end ();
	    
	    p2.begin (&next_text_im);
	    p2.setFont (font2);
	    base_point = QPoint (0, 0);
	    for (int i = 0; i < 2; ++i) {
		p2.setPen (QColor (0, 0, 0, 0xcc));
		p2.drawText (QRect (base_point + QPoint (2, 2), estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			     second_text2.mid (i, 1));
		p2.setPen (QColor (0xff, 0xff, 0xfd));
		p2.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			     second_text2.mid (i, 1));
		base_point.setX (base_point.x () + estimated_char_bounding_size.width ()*0.75);
	    }
	    p2.end ();

	    p2.begin (&mask_im);
	    p2.setRenderHint (QPainter::Antialiasing, true);
	    p2.setPen (Qt::NoPen);
	    p2.setBrush (QColor (0xff, 0xff, 0xff));
	    if (!sec_phase)
		animation_center = QPoint (next_text_im.width ()*double (qrand ())/RAND_MAX, next_text_im.height ()*double (qrand ())/RAND_MAX);
	    QPoint center = animation_center;
	    double radius = double (next_text_im.width ())*sec_phase/10.0;
	    p2.drawPie (QRect (center.x () - radius, center.y () - radius, radius*2, radius*2), 0, 5760);
	    p2.end ();
	    
	    p2.begin (&next_text_im);
	    p2.setCompositionMode (QPainter::CompositionMode_DestinationOut);
	    p2.drawImage (mask_im.rect (), mask_im, mask_im.rect ());
	    p2.end ();

	    mask_im.invertPixels (QImage::InvertRgba);
	    p2.begin (&current_text_im);
	    p2.setCompositionMode (QPainter::CompositionMode_DestinationOut);
	    p2.drawImage (mask_im.rect (), mask_im, mask_im.rect ());
	    p2.end ();

	    base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*2)/2 + estimated_separator_bounding_size.width (),
				 current_rect.height () - estimated_char_bounding_size.height ()*(0.75 + 0.125 + 0.125*0.3)
				 - estimated_arrow_bounding_size.width ());

	    p.drawImage (QRect (base_point, im_size), current_text_im, current_text_im.rect ());

	    base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*2)/2 + estimated_separator_bounding_size.width (),
				 current_rect.height () - estimated_char_bounding_size.height ()*(0.75 + 0.125 + 0.125*0.3)
				 - estimated_arrow_bounding_size.width ());

	    p.drawImage (QRect (base_point, im_size), next_text_im, next_text_im.rect ());

	    p.restore ();
	} else {
	    p.save ();
	    font2.setPixelSize (estimated_font_size*0.75);
	    p.setFont (font2);
	    base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*2)/2 + estimated_separator_bounding_size.width (),
				 current_rect.height () - estimated_char_bounding_size.height ()*(0.75 + 0.125 + 0.125*0.3)
				 - estimated_arrow_bounding_size.width ());
	    for (int i = 0; i < 2; ++i) {
		p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
			    second_text.mid (i, 1));
		base_point.setX (base_point.x () + estimated_char_bounding_size.width ()*0.75);
	    }
	    p.restore ();
	}
	base_point = QPoint (current_rect.width ()/2,
			     current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	if (edit_mode.isEnabled ()) {
	    QColor basic_color (0xd5, 0x92, 0x4b);
	    QColor pressed_color (0x16, 0x83, 0x87);
	    {
		QTransform tr;
		tr.translate (base_point.x (), base_point.y ());
		tr.scale (0.75, 0.75);
		tr.translate (-estimated_arrow_bounding_size.height ()/2, 0);
		tr.rotate (-90);
		tr.translate (-base_point.x (), -base_point.y ());
		p.setTransform (tr);
		p.setPen (((button_pressed == AddMinutePressed) || (button_pressed == ScrollMinutePressed)) ? pressed_color : basic_color);
		p.drawText (QRect (base_point, estimated_arrow_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ">");
	    }
	    {
		QTransform tr;
		tr.translate (base_point.x (), base_point.y ());
		tr.scale (0.75, 0.75);
		tr.translate (-estimated_arrow_bounding_size.height ()/2,
			      estimated_char_bounding_size.height () + estimated_arrow_bounding_size.width ());
		tr.rotate (-90);
		tr.scale (-1.0, 1.0);
		tr.translate (-base_point.x (), -base_point.y ());
		p.setTransform (tr);
		p.setPen (((button_pressed == SubtractMinutePressed) || (button_pressed == ScrollMinutePressed)) ? pressed_color : basic_color);
		p.drawText (QRect (base_point, estimated_arrow_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ">");
	    }
	}
    } break;
    }
    if (KITCHENTIMER_SHOW_DEBUG_OVERLAY) {
	p.save ();
	p.resetTransform ();
	p.setPen (Qt::NoPen);
	p.setBrush (QColor (0xff, 0x88, 0, 60));
	p.drawRect (estimated_minute_add_rect);
	p.setBrush (QColor (0, 0x88, 0xff, 60));
	p.drawRect (estimated_minute_subtract_rect);
	p.setBrush (QColor (0, 0x00, 0xff, 60));
	p.drawRect (estimated_minute_scroll_rect);
	p.restore ();
    }
}
