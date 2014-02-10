#include <QPainter>
#include <QMouseEvent>
#include <QtCore/qmath.h>
#include <QApplication>
#include <QPicture>

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


DigitalTimer::DigitalTimer (QWidget *parent)
    : QWidget (parent), font (qApp->font ()),
      font2 (qApp->font ()),
      value (0, 0, 0), edit_mode (false), unblock_timeout (0), button_pressed (NonePressed), button_pressed_rect (0, 0, 0, 0),
      estimated_size (-1, -1),
      estimated_font_size (3),
      estimated_char_bounding_size (-1, -1),
      estimated_arrow_bounding_size (-1, -1),
      estimated_separator_bounding_size (-1, -1),
      estimated_min_scroll_offset (MIN_OFFSET_TO_START_SCROLL_MIN_PIXELS),
      estimated_scroll_step_offset (SCROLL_STEP_MIN_PIXELS),
      estimated_minute_add_rect (0, 0, 0, 0),
      estimated_minute_subtract_rect (0, 0, 0, 0)
{
    edit_mode_elapsed_timer.invalidate ();
    font.setPixelSize (estimated_font_size);
    setFont (font);
    connect (&repaint_timer, SIGNAL (timeout ()), this, SLOT (checkUpdate ()));
    connect (app_manager->getCurrentTimer (), SIGNAL (newTimeSet ()), this, SLOT (update ()));
}
DigitalTimer::~DigitalTimer ()
{
}
void DigitalTimer::setTime (const QTime &new_value)
{
    value = new_value;
    update ();
}
void DigitalTimer::updateTime ()
{
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
    case 2: {
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
	emit lmb_pressed ();
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
	emit lmb_released ();
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
QSize DigitalTimer::separatorBoundingSize (QPainter &p)
{
    return p.boundingRect (QRect (0, 0, 0, 0), TEXT_FLAGS, ":").size ();
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
	QString hour_text = value.toString ("hh");
	QString minute_text = value.toString ("mm");
	QString second_text = value.toString ("ss");
	QColor text_color (QColor (0xff, 0xff, 0xfd));
	QColor shadow_color (0xbb, 0x9b, 0x79);
	p.setPen (text_color);

	const QRect &current_rect = rect ();
	int secs = QTime (0, 0, 0).secsTo (value);
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
	if (!edit_mode) {
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
    case 2: {
	QString hour_text = value.toString ("hh");
	QString minute_text = value.toString ("mm");
	QString second_text = value.toString ("ss");
	QColor text_color (QColor (0xff, 0xff, 0xfd));
	QColor shadow_color (0xbb, 0x9b, 0x79);
	p.setPen (text_color);

	const QRect &current_rect = rect ();
	int secs = QTime (0, 0, 0).secsTo (value);
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
	if (!edit_mode) {
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
    case 3: {
	QString hour_text = value.toString ("h");
	QString minute_text = value.toString ("mm");
	QString second_text = value.toString ("ss");
	p.setPen (QColor (0xff, 0xff, 0xfd));

	const QRect &current_rect = rect ();
	int secs = QTime (0, 0, 0).secsTo (value);
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
    case 4: {
	int sec_phase = (QTime (0, 0, 0).msecsTo (value)/100)%10;
	QString hour_text = value.toString ("h");
	QString minute_text = value.toString ("mm");
	QString second_text = value.toString ("ss");
	p.setPen (QColor (0xff, 0xff, 0xfd));

	const QRect &current_rect = rect ();
	int secs = QTime (0, 0, 0).secsTo (value);
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
	if (!edit_mode) { // Tough animation should be here
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
