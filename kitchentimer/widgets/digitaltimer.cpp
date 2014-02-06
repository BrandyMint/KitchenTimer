#include <QPainter>
#include <QMouseEvent>
#include <QtCore/qmath.h>
#include <QApplication>
#include <QPicture>

#include "digitaltimer.h"

#define MAX_FONT_SIZE 600
#define MIN_FONT_SIZE 4

#define MAX_MIN (6*60 - 1)
#define MAX_SEC (MAX_MIN*60)
#define MAX_MSEC (MAX_SEC*1000)

#define MIN_OFFSET_TO_START_SCROLL_FRACTION 0.5
#define MIN_OFFSET_TO_START_SCROLL_MIN_PIXELS 10
#define SCROLL_STEP_FRACTION 0.05
#define SCROLL_STEP_MIN_PIXELS 3

#define TEXT_FLAGS (Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip | Qt::TextIncludeTrailingSpaces)


DigitalTimer::DigitalTimer (QWidget *parent)
    : QWidget (parent), font (qApp->font ()),
      font2 (qApp->font ()),
      value (0, 0, 0), edit_mode (false), unblock_timeout (0), button_pressed (NonePressed), button_pressed_rect (0, 0, 0, 0),
      estimated_size (-1, -1),
      estimated_font_size (3),
      estimated_char_bounding_size (-1, -1),
      estimated_arrow_bounding_size (-1, -1),
      estimated_min_scroll_offset (MIN_OFFSET_TO_START_SCROLL_MIN_PIXELS),
      estimated_scroll_step_offset (SCROLL_STEP_MIN_PIXELS),
      estimated_minute_add_rect (0, 0, 0, 0),
      estimated_minute_subtract_rect (0, 0, 0, 0)
{
    edit_mode_elapsed_timer.invalidate ();
    font.setPixelSize (estimated_font_size);
    setFont (font);
    connect (&repaint_timer, SIGNAL (timeout ()), this, SLOT (checkUpdate ()));
}
DigitalTimer::~DigitalTimer ()
{
}
void DigitalTimer::setTime (const QTime &new_value)
{
    value = new_value;
    estimated_size = QSize (-1, -1);
    update ();
}
bool DigitalTimer::addMinuteSharp ()
{
    int ms = QTime (0, 0, 0).msecsTo (value); // TODO: Switch to Qt-5.2.0: .msecsSinceStartOfDay ();
    if (ms == (MAX_SEC*1000))
	return false;
    if (ms%60000)
	ms = ms - ms%60000 + 60000;
    ms += 60000;
    if (ms > (MAX_SEC*1000))
	ms = MAX_SEC*1000;
    value = QTime (0, 0, 0).addMSecs (ms); // TODO: Switch to Qt-5.2.0: QTime::fromMSecsSinceStartOfDay (ms);
    return true;
}
bool DigitalTimer::subtractMinuteSharp ()
{
    int ms = QTime (0, 0, 0).msecsTo (value); // TODO: Switch to Qt-5.2.0: .msecsSinceStartOfDay ();
    if (ms == 0)
	return false;
    if (ms%60000)
	ms = ms - ms%60000 + 60000;
    ms -= 60000;
    if (ms < 0)
	ms = 0;
    value = QTime (0, 0, 0).addMSecs (ms); // TODO: Switch to Qt-5.2.0: QTime::fromMSecsSinceStartOfDay (ms);
    return true;
}
void DigitalTimer::enterEditMode (int new_unblock_timeout)
{
    unblock_timeout = new_unblock_timeout;
    edit_mode = true;
    if (unblock_timeout > 0) {
	edit_mode_elapsed_timer.start ();
    } else {
	edit_mode_elapsed_timer.invalidate ();
    }
    update ();
}
void DigitalTimer::leaveEditMode ()
{
    edit_mode = false;
    update ();
}
void DigitalTimer::resizeEvent (QResizeEvent*)
{
    switch (KITCHENTIMER_DIGITAL_TIMER_MODE) {
    case 0: {
	QPainter p (&estimation_picture);
	p.setFont (font);
	QRect fake_rect (0, 0, 0, 0);
	QString text = value.toString ("hh:mm:ss");
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
void DigitalTimer::mouseMoveEvent (QMouseEvent *event)
{
    emit userIsAlive ();
    const QPoint &pos = event->pos ();
    switch (button_pressed) {
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
		if (addMinuteSharp ())
		    emit timeChanged (value);
		vertical_scroll_accum -= estimated_scroll_step_offset;
	    }
	}
	if (vertical_scroll_accum < 0) {
	    while (vertical_scroll_accum <= -estimated_scroll_step_offset) {
		if (subtractMinuteSharp ())
		    emit timeChanged (value);
		vertical_scroll_accum += estimated_scroll_step_offset;
	    }
	}
	update ();
    } break;
    default: {
    }
    }
    previous_point = pos;
}
void DigitalTimer::mousePressEvent (QMouseEvent *event)
{
    emit userIsAlive ();
    if (event->button () == Qt::LeftButton) {
	if (edit_mode) {
	    if (!edit_mode_elapsed_timer.isValid ()) {
		const QPoint &pos = event->pos ();
		if (estimated_minute_add_rect.contains (pos)) {
		    button_pressed = AddMinutePressed;
		    button_pressed_rect = estimated_minute_add_rect;
		} else if (estimated_minute_subtract_rect.contains (pos)) {
		    button_pressed = SubtractMinutePressed;
		    button_pressed_rect = estimated_minute_subtract_rect;
		} else {
		    button_pressed = ScrollMinutePressed;
		    button_pressed_rect = rect ();
		    vertical_scroll_accum = 0;
		}
		button_pressed_point = pos;
		previous_point = pos;
		update ();
	    }
	} else {
	    emit enterEditModeRequested ();
	}
    }
}
void DigitalTimer::mouseReleaseEvent (QMouseEvent *event)
{
    previous_point = QPoint (-1000000, -1000000);
    emit userIsAlive ();
    if (event->button () == Qt::LeftButton) {
	const QPoint &pos = event->pos ();
	if ((button_pressed != NonePressed) && button_pressed_rect.contains (pos)) {
	    switch (button_pressed) {
	    case AddMinutePressed: {
		if (addMinuteSharp ())
		    emit timeChanged (value);
	    } break;
	    case SubtractMinutePressed: {
		if (subtractMinuteSharp ())
		    emit timeChanged (value);
	    } break;
	    default: {
	    } break;
	    }
	}
	button_pressed = NonePressed;
	update ();
    }
}
QSize DigitalTimer::charBoundingSize (QPainter &p)
{
    QRect fake_rect (0, 0, 0, 0);
    QSize max_size = p.boundingRect (fake_rect, TEXT_FLAGS, ":").size ();
    for (int digit = 0; digit < 10; ++digit)
	max_size = max_size.expandedTo (p.boundingRect (fake_rect, TEXT_FLAGS, QString::number (digit)).size ());
    return max_size;
}
QSize DigitalTimer::arrowBoundingSize (QPainter &p)
{
    return p.boundingRect (QRect (0, 0, 0, 0), TEXT_FLAGS, ">").size ();
}
void DigitalTimer::paintEvent (QPaintEvent*)
{
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
	QString text = value.toString ("hh:mm:ss");
	p.setPen (QColor (0xff, 0xff, 0xfd));

	const QRect &current_rect = rect ();
	int secs = QTime (0, 0, 0).secsTo (value);
	text = QString::number (secs/60 + ((secs%60) ? 1 : 0));
	int nums = text.length ();
	while (text.length () < 3)
	    text = " " + text;
	QPoint base_point ((current_rect.width () - estimated_char_bounding_size.width ()*3)/2,
			   current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	if (nums == 2)
	    base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*4)/2,
				 current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	else if (nums == 1)
	    base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*5)/2,
				 current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	for (int i = 0; i < 3; ++i) {
	    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, text.mid (i, 1));
	    base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
	}
	if (!edit_mode) {
	    p.save ();
	    font2.setPixelSize (estimated_font_size*0.5);
	    p.setFont (font2);
	    p.drawText (QRect (current_rect.x (), current_rect.y (),
			       current_rect.width (), current_rect.height ()),
			Qt::AlignBottom | Qt::AlignHCenter | Qt::TextDontClip, "минуты");
	    p.restore ();
	}
	base_point = QPoint (current_rect.width ()/2,
			     current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	if (edit_mode) {
	    int alpha = 0xff;
	    if (edit_mode_elapsed_timer.isValid ()) {
		int elapsed = edit_mode_elapsed_timer.elapsed ();
		if (elapsed >= unblock_timeout) {
		    edit_mode_elapsed_timer.invalidate ();
		} else {
		    alpha = (0xff*elapsed)/unblock_timeout;
		    alpha = (alpha > 0xff) ? 0xff : alpha;
		}
	    }
	    QColor basic_color (0xd5, 0x92, 0x4b, alpha);
	    QColor pressed_color (0x16, 0x83, 0x87, alpha);
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
	p.restore ();
    }
}
void DigitalTimer::checkUpdate ()
{
    repaint ();
    if (edit_mode_elapsed_timer.isValid () && (edit_mode_elapsed_timer.elapsed () < unblock_timeout)) {
	repaint_timer.start (KITCHENTIMER_ANIMATION_REPAINT_TIMEOUT_MS);
    } else {
	edit_mode_elapsed_timer.invalidate ();
	update ();
    }
}
