#include <QPainter>
#include <QMouseEvent>
#include <QtCore/qmath.h>
#include <QApplication>
#include <QPicture>
#include <QPropertyAnimation>
#include <QScroller>
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
      unblock_timeout (0), down (false), mouse_scroll_step_factor (1.0),
      button_pressed (NonePressed), button_pressed_rect (0, 0, 0, 0), button_pressed_point (-1000000, -1000000),
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
#if 0
    QScroller::grabGesture (this, QScroller::LeftMouseButtonGesture);
#endif
    font.setPixelSize (estimated_font_size);
    setFont (font);
    mouse_move_elapsed_timer.start ();
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
bool DigitalTimer::event (QEvent *event)
{
#if 0
    switch (event->type ()) {
    case QEvent::ScrollPrepare: {
        QScroller *scroller = QScroller::scroller(this);
        // scroller->setSnapPositionsY( WHEEL_SCROLL_OFFSET, rect ().height () );

        QScrollPrepareEvent *scroll_preapare_event = static_cast<QScrollPrepareEvent *>(event);
        scroll_preapare_event->setViewportSize (QSize (2, 2));
        scroll_preapare_event->setContentPosRange (QRect (0, 0, 2, 2));
        scroll_preapare_event->setContentPos (QPoint (1, 1));
        scroll_preapare_event->accept ();
    } return true;
    case QEvent::Scroll: {
    	if (edit_mode.isEnabled () && !edit_mode.isBlocked () && !edit_mode.isHalted ()) {
    	    emit userIsAlive ();
    	    QScrollEvent *scroll_event = static_cast<QScrollEvent *> (event);
    	    double dy = -scroll_event->overshootDistance ().y ()*20;

    	    vertical_scroll_accum -= dy;

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
    	}
    } return true;
    default: {
    }
    }
#endif
    return QWidget::event (event);
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
	    estimated_min_scroll_offset = qMax (double (MIN_OFFSET_TO_START_SCROLL_FRACTION*estimated_char_bounding_size.height ()),
						double (MIN_OFFSET_TO_START_SCROLL_MIN_PIXELS));
	    estimated_scroll_step_offset = qMax (double (SCROLL_STEP_FRACTION*estimated_char_bounding_size.height ()),
						 double (SCROLL_STEP_MIN_PIXELS));
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
    int mouse_move_elapsed_ms = mouse_move_elapsed_timer.elapsed ();
    if (mouse_move_elapsed_ms < 200) {
	mouse_scroll_step_factor = qMax (mouse_scroll_step_factor - 0.1/(1000.0/mouse_move_elapsed_ms), 0.02);
    } else {
	mouse_scroll_step_factor = 1.0;
    }
    mouse_move_elapsed_timer.start ();
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
	int dy = pos.y () - previous_point.y ();
	if (previous_point.y () > -1000000)
	    vertical_scroll_accum -= dy;
	double scroll_step = qMax (estimated_scroll_step_offset*mouse_scroll_step_factor, 0.1);
	if (vertical_scroll_accum > 0) {
	    while (vertical_scroll_accum >= scroll_step) {
		addMinuteSharp ();
		vertical_scroll_accum -= scroll_step;
	    }
	}
	if (vertical_scroll_accum < 0) {
	    while (vertical_scroll_accum <= -scroll_step) {
		subtractMinuteSharp ();
		vertical_scroll_accum += scroll_step;
	    }
	}
    } break;
    default: {
	button_pressed_point = pos;
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
	QString next_second_text = QString ().sprintf ("%02d", (value.second () + 1)%60);
	QColor text_color (QColor (0xff, 0xff, 0xfd));
	QColor shadow_color (0xbb, 0x9b, 0x79);
	p.setPen (text_color);

	QPoint shadow_offset = QPoint (estimated_char_bounding_size.height ()*0.03, estimated_char_bounding_size.height ()*0.045);
	const QRect &current_rect = rect ();
	int base_y_start = current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ();
	if (timer->getPeriod ().isValid () && !edit_mode.isEnabled ()) { // Period
	    double phase = qMin (timer->getMSElapsed ()/2000.0, 1.0);
	    QEasingCurve curve (QEasingCurve::OutExpo);
	    phase = curve.valueForProgress (phase);
	    if (!timer->isRunning ()) {
		phase = 1.0;
	    }
	    p.save ();
	    p.setFont (font);
	    int base_y_start = current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ();
	    QTime value = timer->getPeriod ();
	    QString hour_text = value.toString ("hh");
	    QString minute_text = value.toString ("mm");
	    QString second_text = value.toString ("ss");
	    QColor shadow_color (0xbb, 0x9b, 0x79, 0xee);
	    p.setPen (shadow_color);
	    QTransform tr;
	    QPoint p1 =
		QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*2)/2, base_y_start) +
		QPoint (estimated_char_bounding_size.width (), estimated_char_bounding_size.height ()/2);
	    tr.translate (p1.x (), p1.y ());
	    tr.scale (1.0 - phase*0.1, 1.0 - phase*0.1);
	    tr.translate (-p1.x (), -p1.y ());
	    tr.translate (0.0, -estimated_char_bounding_size.height ()*phase*0.9);
	    p.setWorldTransform (tr);
	    {
		QPoint base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*6)/2
					    - estimated_separator_bounding_size.width ()*2,
					    base_y_start);
		for (int i = 0; i < 2; ++i) {
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point, estimated_char_bounding_size),
				Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				hour_text.mid (i, 1));
		    base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
		}
	    }
	    {
		QPoint base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*4)/2
					    + estimated_separator_bounding_size.width ()/2,
					    base_y_start);
		p.setCompositionMode (QPainter::CompositionMode_Multiply);
		p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	    }
	    {
		QPoint base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*2)/2,
					    base_y_start);
		for (int i = 0; i < 2; ++i) {
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				minute_text.mid (i, 1));
		    base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
		}
	    }
	    {
		QPoint base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*2)/2
					    - estimated_separator_bounding_size.width ()/2,
					    base_y_start);
		p.setCompositionMode (QPainter::CompositionMode_Multiply);
		p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	    }
	    {
		QPoint base_point = QPoint ((current_rect.width () + estimated_char_bounding_size.width ()*2)/2
					    + estimated_separator_bounding_size.width ()*2,
					    base_y_start);
		for (int i = 0; i < 2; ++i) {
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				second_text.mid (i, 1));
		    base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
		}
	    }
	    p.resetTransform ();
	    p.restore ();
	}
	if (!edit_mode.isEnabled ()) { // "hour min sec"
	    p.save ();

	    // TODO: Remove font2
	    font2.setPixelSize (estimated_font_size*0.5);
	    p.setFont (font2);

	    p.setCompositionMode (QPainter::CompositionMode_Multiply);
	    p.setPen (QColor (0xbb, 0x9b, 0x79));

	    {
		p.drawText (QRect (current_rect.x () - estimated_char_bounding_size.width ()*2 - estimated_separator_bounding_size.width ()*2,
				   current_rect.y (),
				   current_rect.width (),
				   current_rect.height ()),
			    Qt::AlignBottom | Qt::AlignHCenter | Qt::TextDontClip, "час");
	    }

	    {
		p.drawText (QRect (current_rect.x (), current_rect.y (),
				   current_rect.width (), current_rect.height ()),
			    Qt::AlignBottom | Qt::AlignHCenter | Qt::TextDontClip, "мин");
	    }

	    {
		p.drawText (QRect (current_rect.x () + estimated_char_bounding_size.width ()*2 + estimated_separator_bounding_size.width ()*2,
				   current_rect.y (),
				   current_rect.width (),
				   current_rect.height ()),
			    Qt::AlignBottom | Qt::AlignHCenter | Qt::TextDontClip, "сек");
	    }

	    p.resetTransform ();
	    p.restore ();
	}
	{ // Hours
	    QPoint base_point = QPoint ((current_rect.width () - estimated_char_bounding_size.width ()*6)/2
					- estimated_separator_bounding_size.width ()*2,
					base_y_start);
	    for (int i = 0; i < 2; ++i) {
		p.setPen (shadow_color);
		p.setCompositionMode (QPainter::CompositionMode_Multiply);
		p.drawText (QRect (base_point + shadow_offset, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
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
	    p.drawText (QRect (base_point + shadow_offset, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
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
		p.drawText (QRect (base_point + shadow_offset, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
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
	    p.drawText (QRect (base_point + shadow_offset, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	    p.setPen (text_color);
	    p.setCompositionMode (QPainter::CompositionMode_SourceOver);
	    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip, ":");
	}
	if (timer->isRunning ()) {
	    int msec = value.msec ();
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
		    tr.translate (estimated_char_bounding_size.width ()*(1.0 - scale)*0.5, 0.0);
		    p.setWorldTransform (tr);
		    p.setPen (c1);
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point + shadow_offset, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
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
		    tr.translate (-estimated_char_bounding_size.width ()*(1.0 - scale)*0.5, 0.0);
		    p.setWorldTransform (tr);
		    p.setPen (c1);
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point + shadow_offset, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
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
		    tr.translate (estimated_char_bounding_size.width ()*(1.0 - scale)*0.5, 0.0);
		    p.setWorldTransform (tr);
		    p.setPen (shadow_color);
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point + shadow_offset, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
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
		    tr.translate (-estimated_char_bounding_size.width ()*(1.0 - scale)*0.5, 0.0);
		    p.setWorldTransform (tr);
		    p.setPen (shadow_color);
		    p.setCompositionMode (QPainter::CompositionMode_Multiply);
		    p.drawText (QRect (base_point + shadow_offset, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
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
		    p.drawText (QRect (base_point + shadow_offset, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				second_text.mid (i, 1));
		    p.setPen (text_color);
		    p.setCompositionMode (QPainter::CompositionMode_SourceOver);
		    p.drawText (QRect (base_point, estimated_char_bounding_size), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextDontClip,
				second_text.mid (i, 1));
		    base_point.setX (base_point.x () + estimated_char_bounding_size.width ());
		}
	    }
	}
	if (edit_mode.isEnabled () && !edit_mode.isBlocked ()) {
	    QPoint base_point = QPoint (current_rect.width ()/2,
					current_rect.height () - estimated_char_bounding_size.height () - estimated_arrow_bounding_size.width ());
	    QColor basic_color (0xd5, 0x92, 0x4b);
	    // TODO: Choose good color
	    // QColor pressed_color (0x16, 0x83, 0x87);
	    // QColor pressed_color (0x7b, 0x54, 0x2a);
	    QColor pressed_color (0xdc, 0xd0, 0xc5);
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
