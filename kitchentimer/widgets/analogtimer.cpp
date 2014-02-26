#include <QPainter>
#include <QMouseEvent>
#include <QtCore/qmath.h>
#include <QApplication>
#include <QTimer>

#include "analogtimer.h"
#include "resourcemanager.h"
#include "applicationmanager.h"

#define MAX_TIME (QTime (5, 59, 0))
#define MAX_SEC ((6*60 - 1)*60)

#define MAX_FOLLOW_ANGLE_EXTENT (KITCHENTIMER_ANALOG_TIMER_MAX_FOLLOW_EXTENT/360.0)

static QRectF getScaledRect (const QRectF &input_rect, double scale_factor)
{
    return QRectF (QPointF (input_rect.x () + (1 - scale_factor)*0.5*input_rect.width (),
			    input_rect.y () + (1 - scale_factor)*0.5*input_rect.height ()),
		   QSizeF (input_rect.width ()*scale_factor,
			   input_rect.height ()*scale_factor));
}
static QRectF getScaledRect (const QRectF &input_rect, double horizontal_scale_factor, double vertical_scale_factor)
{
    return QRectF (QPointF (input_rect.x () + (1 - horizontal_scale_factor)*0.5*input_rect.width (),
			    input_rect.y () + (1 - vertical_scale_factor)*0.5*input_rect.height ()),
		   QSizeF (input_rect.width ()*horizontal_scale_factor,
			   input_rect.height ()*vertical_scale_factor));
}
static double positiveFract (double value)
{
    return value - floor (value);
}
static double shortestRotation (double r1, double r2)
{
    double r = fabs (r1 - r2);
    if (r < 0.5) {
	return r;
    } else {
	return 1.0 - r;
    }
}
static double shortestRotation (double r1, double r2, bool &clock_wise)
{
    double r = fabs (r1 - r2);
    if (r < 0.5) {
	clock_wise = r1 < r2;
	return r;
    } else {
	clock_wise = r1 > r2;
	return 1.0 - r;
    }
}


#define ANIMATION_MIN_FRAME_TIMEOUT 250

AnalogTimer::AnalogTimer (QWidget *parent)
    : QWidget (parent),
      font (qApp->font ()),
      small_font (qApp->font ()),
      animator (this, ANIMATION_MIN_FRAME_TIMEOUT),
      down (false),
      pressed_time (0, 0, 0),
      pressed_local_rotation (0.0),
      previous_delta_rotation (0.0),
      unhalt_by_timeout_time (0, 0, 0),
      unhalt_by_timeout_local_rotation (0.0),
      estimated_circle_rect (0, 0, 0, 0),
      estimated_circle_center (0.0, 0.0),
      estimated_circle_radius (0.0),
      estimated_full_hours (-1)
{
    lifetime_elapsed_timer.start ();
    connect (app_manager->getCurrentTimer (), SIGNAL (timeout ()), &animator, SLOT (stop ()));
    connect (app_manager->getCurrentTimer (), SIGNAL (newTimeSet ()), this, SLOT (update ()));
    connect (&edit_mode, SIGNAL (unhaltedByTimeout ()), this, SLOT (unhaltEditByTimeout ()));
}
AnalogTimer::~AnalogTimer ()
{
}
void AnalogTimer::enterEditMode (int unblock_timeout)
{
    edit_mode.enter (unblock_timeout);
    animator.stop ();
}
void AnalogTimer::enterEditModePressed (int unblock_timeout, int unhalt_timeout)
{
    edit_mode.enterPressed (unblock_timeout, unhalt_timeout);
    if (unblock_timeout > 0) {
	down = true;
	pressed_time = app_manager->getCurrentTimer ()->getTimeLeft ();
	pressed_local_rotation = 0.0;
	previous_delta_rotation = 0.0;
    }
    animator.stop ();
}
void AnalogTimer::leaveEditMode ()
{
    edit_mode.leave ();
    int ms = QTime (0, 0, 0).msecsTo (app_manager->getCurrentTimer ()->getTimeLeft ()); // TODO: Switch to Qt-5.2.0: .msecsSinceStartOfDay ();
    if (ms > 0)
	animator.start ();
    update ();
}
bool AnalogTimer::isSliderDown ()
{
    return down;
}
double AnalogTimer::getRotationByPos (const QPoint &point_pos, const QSize &circle_size)
{
    QPoint rel_pos = point_pos - QPoint (circle_size.width () >> 1, circle_size.height () >> 1);
    return 0.5 - qAtan2 (rel_pos.x (), rel_pos.y ())*0.1591549430919;
}
void AnalogTimer::unhaltEditByTimeout ()
{
    pressed_time = unhalt_by_timeout_time;
    pressed_local_rotation = unhalt_by_timeout_local_rotation;
    previous_delta_rotation = 0.0;
    if (down) {
	{
	    Timer *timer = app_manager->getCurrentTimer ();
	    int int_value = QTime (0, 0, 0).secsTo (timer->getTimeLeft ());
	    int int_angle = int_value%3600;
	    int_angle = ((int_angle + 59)/60)*60;
	    double angle = double (int_angle)/3600.0;
	    if (qAbs (pressed_local_rotation - angle) < MAX_FOLLOW_ANGLE_EXTENT) {
		QTime new_time = QTime (0, 0, 0).addSecs ((int_value/3600)*3600 + 3600*pressed_local_rotation);
		if (new_time.second () < 30) {
		    new_time = new_time.addSecs (-new_time.second ());
		} else {
		    new_time = new_time.addSecs (60 - new_time.second ());
		    if (new_time > MAX_TIME)
			new_time = new_time.addSecs (-60);
		}
		if (timer->getTimeLeft () != new_time) {
		    timer->setTimeLeft (new_time);
		    if (!QTime (0, 0, 0).msecsTo (new_time))
			emit zeroTimeReached ();
		}
		pressed_time = timer->getTimeLeft ();
	    } else if (qAbs (pressed_local_rotation + 1.0 - angle) < MAX_FOLLOW_ANGLE_EXTENT) {
		QTime new_time = QTime (0, 0, 0).addSecs ((int_value/3600)*3600 + 3600*(pressed_local_rotation + 1.0));
		if (new_time.second () < 30) {
		    new_time = new_time.addSecs (-new_time.second ());
		} else {
		    new_time = new_time.addSecs (60 - new_time.second ());
		    if (new_time > MAX_TIME)
			new_time = new_time.addSecs (-60);
		}
		if (timer->getTimeLeft () != new_time) {
		    timer->setTimeLeft (new_time);
		    if (!QTime (0, 0, 0).msecsTo (new_time))
			emit zeroTimeReached ();
		}
		pressed_time = timer->getTimeLeft ();
	    } else if (qAbs (pressed_local_rotation - 1.0 - angle) < MAX_FOLLOW_ANGLE_EXTENT) {
		int new_secs = (int_value/3600)*3600 + 3600*(pressed_local_rotation - 1.0);
		QTime new_time = QTime (0, 0, 0).addSecs (qMax (new_secs, 0));
		if (new_time.second () < 30) {
		    new_time = new_time.addSecs (-new_time.second ());
		} else {
		    new_time = new_time.addSecs (60 - new_time.second ());
		    if (new_time > MAX_TIME)
			new_time = new_time.addSecs (-60);
		}
		if (timer->getTimeLeft () != new_time) {
		    timer->setTimeLeft (new_time);
		    if (!QTime (0, 0, 0).msecsTo (new_time))
			emit zeroTimeReached ();
		}
		pressed_time = timer->getTimeLeft ();
	    }		    
	}
	emit pressed ();
	update ();
    }
}
void AnalogTimer::resizeEvent (QResizeEvent*)
{
    QPoint start_point (0, 0);
    estimated_circle_size = size ();
    if (estimated_circle_size.width () > estimated_circle_size.height ()) {
	start_point.setX ((estimated_circle_size.width () - estimated_circle_size.height ()) >> 1);
	estimated_circle_size.setWidth (estimated_circle_size.height ());
    }
    if (estimated_circle_size.height () > estimated_circle_size.width ()) {
	start_point.setY (0);
	estimated_circle_size.setHeight (estimated_circle_size.width ());
    }
    estimated_circle_rect = QRect (start_point, estimated_circle_size);
    estimated_accum_circle_rect = getScaledRect (estimated_circle_rect, 0.89);
    double scale_factor = double (resource_manager->analog_timer_handle_image.width ())/double (resource_manager->analog_timer_core_image.width ());
    estimated_circle_handle_rect = QRect (int (start_point.x () + estimated_circle_size.width ()*(0.5 - scale_factor*0.5) + 0.5),
					  int (start_point.y () + estimated_circle_size.height ()*(0.5 - scale_factor*0.5) + 0.5),
					  int (estimated_circle_size.width ()*scale_factor) + 0.5,
					  int (estimated_circle_size.height ()*scale_factor) + 0.5);
    estimated_circle_center = QPointF (start_point.x () + estimated_circle_size.width ()*0.5, start_point.y () + estimated_circle_size.height ()*0.5);
    estimated_circle_radius = estimated_circle_rect.width ()*0.5;
    estimated_font_pixel_size = estimated_circle_rect.width ()*0.1;
    font.setPixelSize (estimated_font_pixel_size);
    small_font.setPixelSize (estimated_font_pixel_size*0.75);
    {
	cached_back_layer = QImage (estimated_circle_size, QImage::Format_ARGB32);
	cached_back_layer.fill (0);

	QPainter p;
	p.begin (&cached_back_layer);
	p.setRenderHint (QPainter::SmoothPixmapTransform, true);
	p.drawImage (QRect (QPoint (0, 0), estimated_circle_size), resource_manager->analog_timer_core_image,
		     resource_manager->analog_timer_core_image.rect ());
	p.end ();
    }
    {
	cached_analog_timer_handle_layer = QImage (estimated_circle_handle_rect.size (), QImage::Format_ARGB32);
	cached_analog_timer_handle_layer.fill (0);

	QPainter p;
	p.begin (&cached_analog_timer_handle_layer);
	p.setRenderHint (QPainter::SmoothPixmapTransform, true);
	p.drawImage (QRect (QPoint (0, 0), estimated_circle_handle_rect.size ()), resource_manager->analog_timer_handle_image,
		     resource_manager->analog_timer_handle_image.rect ());
	p.end ();
    }
    {
	cached_over_layer = QImage (estimated_circle_size, QImage::Format_ARGB32);
	cached_over_layer.fill (0);
	
	QPainter p;
	p.begin (&cached_over_layer);
	p.setRenderHint (QPainter::Antialiasing, true);
	p.setRenderHint (QPainter::SmoothPixmapTransform, true);
	p.setPen (Qt::NoPen);
	{
	    {
		p.setBrush (QColor (0xa8, 0x65, 0x2b));
		double angle = 0.0;
		for (int i = 0; i < 12*5; ++i, angle += 6.0) {
		    if (i%5) {
			QTransform tr;
			tr.translate (estimated_circle_center.x () - start_point.x (), estimated_circle_center.y () - start_point.y ());
			tr.rotate (angle);
			tr.translate (-estimated_circle_center.x () + start_point.x (), -estimated_circle_center.y () + start_point.y ());
			p.setWorldTransform (tr);
			p.drawRect (QRectF (estimated_circle_center - start_point + QPointF (-estimated_circle_rect.width ()*0.003,
											     estimated_circle_rect.height ()*0.29),
					    QSizeF (estimated_circle_rect.width ()*0.006, estimated_circle_rect.height ()*0.035)));
		    }
		}
	    }

	    {
		p.setBrush (QColor (0x71, 0x44, 0x1d));
		double angle = 0.0;
		for (int i = 0; i < 12; ++i, angle += 30.0) {
		    QTransform tr;
		    tr.translate (estimated_circle_center.x () - start_point.x (), estimated_circle_center.y () - start_point.y ());
		    tr.rotate (angle);
		    tr.translate (-estimated_circle_center.x () + start_point.x (), -estimated_circle_center.y () + start_point.y ());
		    p.setWorldTransform (tr);
		    p.drawRect (QRectF (estimated_circle_center - start_point + QPointF (-estimated_circle_rect.width ()*0.005, estimated_circle_rect.height ()*0.29),
					QSizeF (estimated_circle_rect.width ()*0.01, estimated_circle_rect.height ()*((i%3) ? 0.035 : 0.06))));
		}
	    }

	    p.resetTransform ();
	    double angle = M_PI*0.5*9.0/3.0;
	    int n = 0;
	    p.setBrush (Qt::NoBrush);
	    for (int i = 0; i < 12; ++i) {
		double text_radius = estimated_circle_radius*((i%3) ? 0.75 : 0.8);
		// TODO: Remove small font
		if (i%3)
		    p.setFont (small_font);
		else
		    p.setFont (font);
		p.setPen (QColor (0xc0, 0, 0));
		p.drawText (QRectF (QPointF (estimated_circle_center - start_point - QPointF (estimated_font_pixel_size*1.5 - text_radius*cos (angle),
											      estimated_font_pixel_size*1.5 - text_radius*sin (angle))),
				    QSizeF (estimated_font_pixel_size*3.0, estimated_font_pixel_size*3.0)), Qt::AlignCenter, QString::number (n));
		angle += M_PI*0.5*1.0/3.0;
		n += 5;
	    }
	}
	p.end ();
    }
#ifdef Q_OS_MAC
    {
	blend_layer = QImage (estimated_circle_size, QImage::Format_ARGB32);
	blend_layer.fill (0);
    }
#endif
    QRectF handle_rect = getScaledRect (estimated_circle_rect, 0.077519379844961, 0.51162790697674);
    estimated_handle_triangle_points[0] = QPointF (handle_rect.x () + handle_rect.width ()*0.1, handle_rect.y () + handle_rect.height ()*0.12);
    estimated_handle_triangle_points[1] = QPointF (handle_rect.x () + handle_rect.width ()*0.9, handle_rect.y () + handle_rect.height ()*0.12);
    estimated_handle_triangle_points[2] = QPointF (handle_rect.x () + handle_rect.width ()*0.5, handle_rect.y () + handle_rect.height ()*0.07);
    estimated_full_hours = -1;
}
void AnalogTimer::mousePressEvent (QMouseEvent *event)
{
    emit userIsAlive ();
    QPointF tmp = estimated_circle_center - event->pos ();
    if (QPointF::dotProduct (tmp, tmp) > estimated_circle_radius*estimated_circle_radius) {
	event->ignore ();
	return;
    }
    emit clearAlarms ();
    if (event->button () == Qt::LeftButton) {
	Timer *timer = app_manager->getCurrentTimer ();
	emit lmb_pressed ();
	QPoint current_pos = event->pos () - estimated_circle_rect.topLeft ();
	QTime current_time = timer->getTimeLeft ();
	unhalt_by_timeout_time = current_time;
	unhalt_by_timeout_local_rotation = getRotationByPos (current_pos, estimated_circle_rect.size ());
	if (edit_mode.isEnabled ()) {
	    if (edit_mode.isBlocked ()) {
		down = true;
	    } else {
		if (!down)
		    emit pressed ();
		down = true;
		pressed_time = current_time;
		pressed_local_rotation = getRotationByPos (current_pos, estimated_circle_rect.size ());
		previous_delta_rotation = 0.0;
		{
		    int int_value = QTime (0, 0, 0).secsTo (timer->getTimeLeft ());
		    int int_angle = int_value%3600;
		    int_angle = ((int_angle + 59)/60)*60;
		    double angle = double (int_angle)/3600.0;
		    if (qAbs (pressed_local_rotation - angle) < MAX_FOLLOW_ANGLE_EXTENT) {
			QTime new_time = QTime (0, 0, 0).addSecs ((int_value/3600)*3600 + 3600*pressed_local_rotation);
			if (new_time.second () < 30) {
			    new_time = new_time.addSecs (-new_time.second ());
			} else {
			    new_time = new_time.addSecs (60 - new_time.second ());
			    if (new_time > MAX_TIME)
				new_time = new_time.addSecs (-60);
			}
			if (timer->getTimeLeft () != new_time) {
			    timer->setTimeLeft (new_time);
			    if (!QTime (0, 0, 0).msecsTo (new_time))
				emit zeroTimeReached ();
			}
			pressed_time = timer->getTimeLeft ();
		    } else if (qAbs (pressed_local_rotation + 1.0 - angle) < MAX_FOLLOW_ANGLE_EXTENT) {
			QTime new_time = QTime (0, 0, 0).addSecs ((int_value/3600)*3600 + 3600*(pressed_local_rotation + 1.0));
			if (new_time.second () < 30) {
			    new_time = new_time.addSecs (-new_time.second ());
			} else {
			    new_time = new_time.addSecs (60 - new_time.second ());
			    if (new_time > MAX_TIME)
				new_time = new_time.addSecs (-60);
			}
			if (timer->getTimeLeft () != new_time) {
			    timer->setTimeLeft (new_time);
			    if (!QTime (0, 0, 0).msecsTo (new_time))
				emit zeroTimeReached ();
			}
			pressed_time = timer->getTimeLeft ();
		    } else if (qAbs (pressed_local_rotation - 1.0 - angle) < MAX_FOLLOW_ANGLE_EXTENT) {
			int new_secs = (int_value/3600)*3600 + 3600*(pressed_local_rotation - 1.0);
			QTime new_time = QTime (0, 0, 0).addSecs (qMax (new_secs, 0));
			if (new_time.second () < 30) {
			    new_time = new_time.addSecs (-new_time.second ());
			} else {
			    new_time = new_time.addSecs (60 - new_time.second ());
			    if (new_time > MAX_TIME)
				new_time = new_time.addSecs (-60);
			}
			if (timer->getTimeLeft () != new_time) {
			    timer->setTimeLeft (new_time);
			    if (!QTime (0, 0, 0).msecsTo (new_time))
				emit zeroTimeReached ();
			}
			pressed_time = timer->getTimeLeft ();
		    }		    
		}
		edit_mode.unhalt ();
		update ();
	    }
	} else {
	    emit enterEditModeRequested ();
	}
    }
}
void AnalogTimer::mouseReleaseEvent (QMouseEvent *event)
{
    emit clearAlarms ();
    if (event->button () == Qt::LeftButton) {
	emit lmb_released ();
	if (down) {
	    if (edit_mode.isEnabled () && !edit_mode.isHalted ())
		emit released ();
	    down = false;
	    QPointF tmp = estimated_circle_center - event->pos ();
	    if (QPointF::dotProduct (tmp, tmp) <= estimated_circle_radius*estimated_circle_radius) {
		if (edit_mode.isEnabled () && !edit_mode.isBlocked ())
		    emit leaveEditModeRequested ();
	    }
	    update ();
	}
    }
}
void AnalogTimer::mouseMoveEvent (QMouseEvent *event)
{
    emit userIsAlive ();
    emit clearAlarms ();
    if (down) {
	if (edit_mode.isEnabled () && !edit_mode.isBlocked ()) {
	    Timer *timer = app_manager->getCurrentTimer ();
	    QPoint current_pos = event->pos () - estimated_circle_rect.topLeft ();
	    double blind_circle_radius = estimated_circle_rect.width ()*0.05;
	    QPointF tmp = estimated_circle_center - event->pos ();
	    if (QPointF::dotProduct (tmp, tmp) < blind_circle_radius*blind_circle_radius) {
		if (!edit_mode.isHalted ()) {
		    edit_mode.halt ();
		    emit released ();
		    update ();
		}
	    } else if (edit_mode.isHalted ()) {
		if (edit_mode.isHaltedByTimeout ()) {
		    unhalt_by_timeout_time = timer->getTimeLeft ();
		    unhalt_by_timeout_local_rotation = getRotationByPos (current_pos, estimated_circle_rect.size ());
		} else {
		    pressed_time = timer->getTimeLeft ();
		    pressed_local_rotation = getRotationByPos (current_pos, estimated_circle_rect.size ());
		    previous_delta_rotation = 0.0;
		    {
			int int_value = QTime (0, 0, 0).secsTo (timer->getTimeLeft ());
			int int_angle = int_value%3600;
			int_angle = ((int_angle + 59)/60)*60;
			double angle = double (int_angle)/3600.0;
			if (qAbs (pressed_local_rotation - angle) < MAX_FOLLOW_ANGLE_EXTENT) {
			    QTime new_time = QTime (0, 0, 0).addSecs ((int_value/3600)*3600 + 3600*pressed_local_rotation);
			    if (new_time.second () < 30) {
				new_time = new_time.addSecs (-new_time.second ());
			    } else {
				new_time = new_time.addSecs (60 - new_time.second ());
				if (new_time > MAX_TIME)
				    new_time = new_time.addSecs (-60);
			    }
			    if (timer->getTimeLeft () != new_time) {
				timer->setTimeLeft (new_time);
				if (!QTime (0, 0, 0).msecsTo (new_time))
				    emit zeroTimeReached ();
			    }
			    pressed_time = timer->getTimeLeft ();
			} else if (qAbs (pressed_local_rotation + 1.0 - angle) < MAX_FOLLOW_ANGLE_EXTENT) {
			    QTime new_time = QTime (0, 0, 0).addSecs ((int_value/3600)*3600 + 3600*(pressed_local_rotation + 1.0));
			    if (new_time.second () < 30) {
				new_time = new_time.addSecs (-new_time.second ());
			    } else {
				new_time = new_time.addSecs (60 - new_time.second ());
				if (new_time > MAX_TIME)
				    new_time = new_time.addSecs (-60);
			    }
			    if (timer->getTimeLeft () != new_time) {
				timer->setTimeLeft (new_time);
				if (!QTime (0, 0, 0).msecsTo (new_time))
				    emit zeroTimeReached ();
			    }
			    pressed_time = timer->getTimeLeft ();
			} else if (qAbs (pressed_local_rotation - 1.0 - angle) < MAX_FOLLOW_ANGLE_EXTENT) {
			    int new_secs = (int_value/3600)*3600 + 3600*(pressed_local_rotation - 1.0);
			    QTime new_time = QTime (0, 0, 0).addSecs (qMax (new_secs, 0));
			    if (new_time.second () < 30) {
				new_time = new_time.addSecs (-new_time.second ());
			    } else {
				new_time = new_time.addSecs (60 - new_time.second ());
				if (new_time > MAX_TIME)
				    new_time = new_time.addSecs (-60);
			    }
			    if (timer->getTimeLeft () != new_time) {
				timer->setTimeLeft (new_time);
				if (!QTime (0, 0, 0).msecsTo (new_time))
				    emit zeroTimeReached ();
			    }
			    pressed_time = timer->getTimeLeft ();
			}		    
		    }
		    edit_mode.unhalt ();
		    emit pressed ();
		}
	    } else {
		double local_rotation = getRotationByPos (current_pos, estimated_circle_rect.size ());
		double previous_local_rotation = positiveFract (pressed_local_rotation + previous_delta_rotation);
		bool clockwise, clockwise2;
		double d = shortestRotation (previous_local_rotation, local_rotation, clockwise);
		double shp1 = shortestRotation (pressed_local_rotation, previous_local_rotation);
		double shp2 = shortestRotation (pressed_local_rotation, local_rotation, clockwise2);
		bool overlap = (shp1 < d) && (shp2 < d);
		if (clockwise) {
		    previous_delta_rotation = floor (previous_delta_rotation) + (clockwise2 ? shp2 : (1.0 - shp2)) + (overlap ? 1.0 : 0.0);
		} else {
		    previous_delta_rotation = ceil (previous_delta_rotation) + (clockwise2 ? (shp2 - 1.0) : -shp2) - (overlap ? 1.0 : 0.0);
		}
		int pressed_sec = QTime (0, 0, 0).secsTo (pressed_time) + previous_delta_rotation*3600;
		if (pressed_sec < 0) {
		    pressed_sec = 0;
		    pressed_time = QTime (0, 0, 0);
		    pressed_local_rotation = local_rotation;
		    previous_delta_rotation = -0.00001;
		} else if (pressed_sec > MAX_SEC) {
		    pressed_sec = MAX_SEC;
		    pressed_time = MAX_TIME;
		    pressed_local_rotation = local_rotation;
		    previous_delta_rotation = 0.00001;
		}
		QTime new_time = QTime (0, 0, 0).addSecs (pressed_sec);
		if (new_time.second () < 30) {
		    new_time = new_time.addSecs (-new_time.second ());
		} else {
		    new_time = new_time.addSecs (60 - new_time.second ());
		    if (new_time > MAX_TIME)
			new_time = new_time.addSecs (-60);
		}
		if (timer->getTimeLeft () != new_time) {
		    timer->setTimeLeft (new_time);
		    if (!QTime (0, 0, 0).msecsTo (new_time))
			emit zeroTimeReached ();
		    emit slide ();
		}
	    }
	}
    }
}
void AnalogTimer::paintEvent (QPaintEvent*)
{
    QPainter p (this);
    if (KITCHENTIMER_SHOW_DEBUG_OVERLAY) {
	p.setBrush (QColor (0xff, 0, 0, 60));
	p.drawRect (rect ());
    }

    switch (KITCHENTIMER_ANALOG_TIMER_MODE) {
    case 0: {
	p.setFont (font);
	p.setRenderHint (QPainter::Antialiasing, true);
	p.setRenderHint (QPainter::SmoothPixmapTransform, true);
	p.setPen (Qt::NoPen);
	int int_value = QTime (0, 0, 0).secsTo (app_manager->getCurrentTimer ()->getTimeLeft ());
	int int_angle = int_value%3600;
	int_angle = ((int_angle + 59)/60)*60;
	double angle = double (int_angle)*0.1;

	bool draw_down = down && !edit_mode.isHalted ();
	double press_scale_factor = draw_down ? 0.97 : 1.0;

	QColor core_color = draw_down ? QColor (0xe8, 0xe8, 0xe8) : QColor (0xfd, 0xfd, 0xfd);

	{
	    p.setBrush (QColor (0, 0, 0, 80));
	    p.drawPie (estimated_circle_rect, 0, 5760);
	    p.setBrush (QColor (0xf3, 0xf3, 0xf3));
	    p.drawPie (getScaledRect (estimated_circle_rect, 0.96899224806202), 0, 5760);
	    p.setBrush (QColor (0xff, 0xff, 0xff));
	    p.drawPie (getScaledRect (estimated_circle_rect, 0.89147286821705), 0, 5760);
	    p.setBrush (QColor (0xc0, 0x33, 0x33));
	    p.drawPie (getScaledRect (estimated_circle_rect, 0.6), 1440, -(int_angle << 3)/5);
	    p.setBrush (QColor (0xcc, 0xcc, 0xcc));
	    p.drawPie (getScaledRect (estimated_circle_rect, 0.55813953488372), 0, 5760);
	    if (draw_down) {
		p.setBrush (QColor (0xc3, 0xc3, 0xc3));
		p.drawPie (getScaledRect (estimated_circle_rect, 0.53488372093023), 0, 5760);
	    }
	    p.setBrush (core_color);
	    p.drawPie (getScaledRect (estimated_circle_rect, 0.53488372093023*press_scale_factor), 0, 5760);
	}

	{
	    p.setBrush (QColor (180, 180, 180));
	    double angle = 0.0;
	    for (int i = 0; i < 12*5; ++i, angle += 6.0) {
		if (i%5) {
		    QTransform tr;
		    tr.translate (estimated_circle_center.x (), estimated_circle_center.y ());
		    tr.rotate (angle);
		    tr.translate (-estimated_circle_center.x (), -estimated_circle_center.y ());
		    p.setWorldTransform (tr);
		    p.drawRect (QRectF (estimated_circle_center + QPointF (-estimated_circle_rect.width ()*0.003,
									   estimated_circle_rect.height ()*0.29),
					QSizeF (estimated_circle_rect.width ()*0.006, estimated_circle_rect.height ()*0.035)));
		}
	    }
	}

	{
	    p.setBrush (QColor (20, 20, 20));
	    double angle = 0.0;
	    for (int i = 0; i < 12; ++i, angle += 30.0) {
		QTransform tr;
		tr.translate (estimated_circle_center.x (), estimated_circle_center.y ());
		tr.rotate (angle);
		tr.translate (-estimated_circle_center.x (), -estimated_circle_center.y ());
		p.setWorldTransform (tr);
		p.drawRect (QRectF (estimated_circle_center + QPointF (-estimated_circle_rect.width ()*0.005, estimated_circle_rect.height ()*0.29),
				    QSizeF (estimated_circle_rect.width ()*0.01, estimated_circle_rect.height ()*0.045)));
	    }
	}

	{
	    QTransform tr;
	    tr.translate (estimated_circle_center.x (), estimated_circle_center.y ());
	    tr.rotate (angle);
	    tr.translate (-estimated_circle_center.x (), -estimated_circle_center.y ());
	    p.setWorldTransform (tr);
	    QRectF handle_shadow_rect = getScaledRect (estimated_circle_rect, 0.137519379844961*press_scale_factor,
						       0.51162790697674*press_scale_factor);
	    QLinearGradient handle_shadow_grad (handle_shadow_rect.topLeft () + QPointF (0.0, handle_shadow_rect.height ()*0.5),
						handle_shadow_rect.topLeft () + QPointF (handle_shadow_rect.width (),
											 handle_shadow_rect.height ()*0.5));
	    handle_shadow_grad.setColorAt (0, core_color);
	    handle_shadow_grad.setColorAt (0.25, QColor (0xc0, 0xc0, 0xc0));
	    handle_shadow_grad.setColorAt (0.75, QColor (0xc0, 0xc0, 0xc0));
	    handle_shadow_grad.setColorAt (1, core_color);
	    p.setBrush (handle_shadow_grad);
	    p.drawRect (handle_shadow_rect);
	    QRectF handle_rect = getScaledRect (estimated_circle_rect, 0.077519379844961*press_scale_factor, 0.51162790697674*press_scale_factor);
	    QLinearGradient handle_grad (handle_rect.topLeft () + QPointF (handle_rect.width ()*0.5, 0.0),
					 handle_rect.topLeft () + QPointF (handle_rect.width ()*0.5, handle_rect.height ()));
	    handle_grad.setColorAt (0, core_color);
	    handle_grad.setColorAt (0.20, QColor (0xc6, 0xc6, 0xc6));
	    handle_grad.setColorAt (0.45, Qt::white);
	    handle_grad.setColorAt (0.55, Qt::white);
	    handle_grad.setColorAt (0.75, QColor (0xc6, 0xc6, 0xc6));
	    handle_grad.setColorAt (1, core_color);
	    p.setBrush (handle_grad);
	    p.drawRect (handle_rect);
	    p.setBrush (QColor (0x85, 0x6b, 0x61));
	    QPointF triangle_points[3] = {
		QPointF (handle_rect.x () + handle_rect.width ()*0.1, handle_rect.y () + handle_rect.height ()*0.1),
		QPointF (handle_rect.x () + handle_rect.width ()*0.9, handle_rect.y () + handle_rect.height ()*0.1),
		QPointF (handle_rect.x () + handle_rect.width ()*0.5, handle_rect.y () + handle_rect.height ()*0.05),
	    };
	    p.drawPolygon (triangle_points, 3);
	}
	p.resetTransform ();
	double text_radius = estimated_circle_radius*0.8;
	p.setPen (QColor (0xc0, 0, 0));
	angle = M_PI*0.5*9.0/3.0;
	int n = 0;
	for (int i = 0; i < 12; ++i) {
	    p.drawText (QRectF (QPointF (estimated_circle_center - QPointF (estimated_font_pixel_size*1.5 - text_radius*cos (angle),
									    estimated_font_pixel_size*1.5 - text_radius*sin (angle))),
				QSizeF (estimated_font_pixel_size*3.0, estimated_font_pixel_size*3.0)), Qt::AlignCenter, QString::number (n));
	    angle += M_PI*0.5*1.0/3.0;
	    n += 5;
	}
    } break;
    case 1: {
	int int_value = QTime (0, 0, 0).secsTo (app_manager->getCurrentTimer ()->getTimeLeft ());
	int int_angle = int_value%3600;
	int_angle = ((int_angle + 59)/60)*60;
	double angle = double (int_angle)*0.1;
	int full_hours = int_value/3600;

	bool draw_down = down && !edit_mode.isHalted ();
	double press_scale_factor = draw_down ? 0.97 : 1.0;

	if (estimated_full_hours != full_hours) {
	    QColor transparent_color (0xff, 0, 0, 0);
	    QColor current_color (0xff, 0, 0, 0x88);
	    QColor current2_color (0xff, 0, 0, 0xcc);
	    QColor current3_color (0xff, 0, 0, 0x66);
	    QColor full_color (0xff, 0, 0, 0x44);
	    double tr_l = 4.0;
	    double current_l = 20.0;
	    double full_l = 10.0;
	    double start = 0.64;
	    double range_f = 1.0 - start;
	    double range_l = (full_hours + 1.0)*2.0 + full_hours*(tr_l + full_l) + current_l;
	    double current_circle_radius = estimated_circle_radius*0.89;
	    estimated_current_grad = QRadialGradient (estimated_circle_center, current_circle_radius);
	    estimated_current_grad.setColorAt (0.0, transparent_color);
	    estimated_current_grad.setColorAt (start, transparent_color);
	    estimated_current_grad.setColorAt (1.0, transparent_color);
	    estimated_current2_grad = QRadialGradient (estimated_circle_center, current_circle_radius);
	    estimated_current2_grad.setColorAt (0.0, transparent_color);
	    estimated_current2_grad.setColorAt (start, transparent_color);
	    estimated_current2_grad.setColorAt (1.0, transparent_color);
	    estimated_current3_grad = QRadialGradient (estimated_circle_center, current_circle_radius);
	    estimated_current3_grad.setColorAt (0.0, transparent_color);
	    estimated_current3_grad.setColorAt (start, transparent_color);
	    estimated_current3_grad.setColorAt (1.0, transparent_color);
	    estimated_full_grad = QRadialGradient (estimated_circle_center, current_circle_radius);
	    estimated_full_grad.setColorAt (0.0, transparent_color);
	    estimated_full_grad.setColorAt (start, transparent_color);
	    estimated_full_grad.setColorAt (1.0, transparent_color);
	    double off = 0.0;
	    estimated_current_grad.setColorAt (start + range_f*(off += 1.0)/range_l, current_color);
	    estimated_current2_grad.setColorAt (start + range_f*(off)/range_l, current2_color);
	    estimated_current3_grad.setColorAt (start + range_f*(off)/range_l, current3_color);
	    estimated_current_grad.setColorAt (start + range_f*(off += current_l)/range_l, current_color);
	    estimated_current2_grad.setColorAt (start + range_f*(off)/range_l, current2_color);
	    estimated_current3_grad.setColorAt (start + range_f*(off)/range_l, current3_color);
	    estimated_current_grad.setColorAt (start + range_f*(off += 1.0)/range_l, transparent_color);
	    estimated_current2_grad.setColorAt (start + range_f*(off)/range_l, transparent_color);
	    estimated_current3_grad.setColorAt (start + range_f*(off)/range_l, transparent_color);
	    estimated_full_grad.setColorAt (start + range_f*off/range_l, transparent_color);
	    for (int i = 0; i < full_hours; ++i) {
		estimated_full_grad.setColorAt (start + range_f*(off += tr_l)/range_l, transparent_color);
		estimated_full_grad.setColorAt (start + range_f*(off += 1.0)/range_l, full_color);
		estimated_full_grad.setColorAt (start + range_f*(off += full_l)/range_l, full_color);
		estimated_full_grad.setColorAt (start + range_f*(off += 1.0)/range_l, transparent_color);
	    }
	    estimated_full_hours = full_hours;
	}
#ifdef Q_OS_MAC
	QPainter p2;
	blend_layer.fill (0);
	p2.begin (&blend_layer);
	p2.drawImage (QRect (QPoint (0, 0), estimated_circle_size), cached_back_layer, cached_back_layer.rect ());
	p2.setRenderHint (QPainter::Antialiasing, true);
	p2.setRenderHint (QPainter::SmoothPixmapTransform, true);
	{
	    QTransform tr;
	    tr.translate (-estimated_circle_rect.x (), -estimated_circle_rect.y ());
	    p2.setWorldTransform (tr);
	    p2.setPen (Qt::NoPen);
	    p2.setBrush (estimated_full_grad);
	    p2.drawPie (estimated_accum_circle_rect, 0, 5760);
	    p2.setBrush (estimated_current_grad);
	    if (app_manager->getCurrentTimer ()->isRunning ()) {
		if (int_angle) {
		    p2.drawPie (estimated_accum_circle_rect, 1440, 96 - angle*16);
		    if (lifetime_elapsed_timer.elapsed ()%500 < 250)
			p2.setBrush (estimated_current2_grad);
		    else
			p2.setBrush (estimated_current3_grad);
		    p2.drawPie (estimated_accum_circle_rect, 1536 - angle*16, -96);
		}
	    } else {
		p2.drawPie (estimated_accum_circle_rect, 1440, -angle*16);
	    }
	    p2.resetTransform ();
	}
	{
	    QTransform tr;
	    tr.translate (-estimated_circle_rect.x (), -estimated_circle_rect.y ());
	    tr.translate (estimated_circle_center.x (), estimated_circle_center.y ());
	    tr.rotate (angle);
	    tr.scale (press_scale_factor, press_scale_factor);
	    tr.translate (-estimated_circle_center.x (), -estimated_circle_center.y ());
	    p2.setWorldTransform (tr);
	    p2.drawImage (estimated_circle_handle_rect, cached_analog_timer_handle_layer,
			  cached_analog_timer_handle_layer.rect ());
	    p2.setPen (Qt::NoPen);
	    p2.setBrush (QColor (0xbf, 0x00, 0x00));
	    p2.drawPolygon (estimated_handle_triangle_points, 3);
	    p2.resetTransform ();
	}
	p2.setRenderHint (QPainter::Antialiasing, false);
	p2.setRenderHint (QPainter::SmoothPixmapTransform, false);
	{
	    QTransform tr;
	    tr.translate (-estimated_circle_rect.x (), -estimated_circle_rect.y ());
	    p2.setWorldTransform (tr);
	    p2.drawImage (estimated_circle_rect, cached_over_layer, cached_over_layer.rect ());
	    p2.resetTransform ();
	}
	p2.end ();
	p.drawImage (estimated_circle_rect, blend_layer, cached_over_layer.rect ());
#else
	p.drawImage (estimated_circle_rect, cached_back_layer, cached_back_layer.rect ());
	p.setRenderHint (QPainter::Antialiasing, true);
	p.setRenderHint (QPainter::SmoothPixmapTransform, true);
	{
	    p.setPen (Qt::NoPen);
	    p.setBrush (estimated_full_grad);
	    p.drawPie (estimated_accum_circle_rect, 0, 5760);
	    p.setBrush (estimated_current_grad);
	    if (app_manager->getCurrentTimer ()->isRunning ()) {
		if (int_angle) {
		    p.drawPie (estimated_accum_circle_rect, 1440, 96 - angle*16);
		    if (lifetime_elapsed_timer.elapsed ()%500 < 250)
			p.setBrush (estimated_current2_grad);
		    else
			p.setBrush (estimated_current3_grad);
		    p.drawPie (estimated_accum_circle_rect, 1536 - angle*16, -96);
		}
	    } else {
		p.drawPie (estimated_accum_circle_rect, 1440, -angle*16);
	    }
	}
	{
	    QTransform tr;
	    tr.translate (estimated_circle_center.x (), estimated_circle_center.y ());
	    tr.rotate (angle);
	    tr.scale (press_scale_factor, press_scale_factor);
	    tr.translate (-estimated_circle_center.x (), -estimated_circle_center.y ());
	    p.setWorldTransform (tr);
	    p.drawImage (estimated_circle_handle_rect, cached_analog_timer_handle_layer,
			 cached_analog_timer_handle_layer.rect ());
	    p.setPen (Qt::NoPen);
	    p.setBrush (QColor (0xbf, 0x00, 0x00));
	    p.drawPolygon (estimated_handle_triangle_points, 3);
	}
	p.resetTransform ();
	p.setRenderHint (QPainter::Antialiasing, false);
	p.setRenderHint (QPainter::SmoothPixmapTransform, false);
	p.drawImage (estimated_circle_rect, cached_over_layer, cached_over_layer.rect ());
#endif
    } break;
    }
}
