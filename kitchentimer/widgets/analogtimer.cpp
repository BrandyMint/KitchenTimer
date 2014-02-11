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
      estimated_circle_radius (0.0)
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
	emit pressed ();
	update ();
    }
}
void AnalogTimer::resizeEvent (QResizeEvent*)
{
    QPoint start_point (0, 0);
    QSize circle_size (size ());
    if (circle_size.width () > circle_size.height ()) {
	start_point.setX ((circle_size.width () - circle_size.height ()) >> 1);
	circle_size.setWidth (circle_size.height ());
    }
    if (circle_size.height () > circle_size.width ()) {
	start_point.setY (0);
	circle_size.setHeight (circle_size.width ());
    }
    estimated_circle_rect = QRect (start_point, circle_size);
    double scale_factor = double (resource_manager->analog_timer_handle_image.width ())/double (resource_manager->analog_timer_core_image.width ());
    estimated_circle_handle_rect = QRect (int (start_point.x () + circle_size.width ()*(0.5 - scale_factor*0.5) + 0.5),
					  int (start_point.y () + circle_size.height ()*(0.5 - scale_factor*0.5) + 0.5),
					  int (circle_size.width ()*scale_factor) + 0.5,
					  int (circle_size.height ()*scale_factor) + 0.5);
    estimated_circle_center = QPointF (start_point.x () + circle_size.width ()*0.5, start_point.y () + circle_size.height ()*0.5);
    estimated_circle_radius = estimated_circle_rect.width ()*0.5;
    estimated_font_pixel_size = estimated_circle_rect.width ()*0.1;
    font.setPixelSize (estimated_font_pixel_size);
    small_font.setPixelSize (estimated_font_pixel_size*0.75);
    setFont (font);
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
	emit lmb_pressed ();
	QPoint current_pos = event->pos () - estimated_circle_rect.topLeft ();
	QTime current_time = app_manager->getCurrentTimer ()->getTimeLeft ();
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
		    unhalt_by_timeout_time = app_manager->getCurrentTimer ()->getTimeLeft ();
		    unhalt_by_timeout_local_rotation = getRotationByPos (current_pos, estimated_circle_rect.size ());
		} else {
		    pressed_time = app_manager->getCurrentTimer ()->getTimeLeft ();
		    pressed_local_rotation = getRotationByPos (current_pos, estimated_circle_rect.size ());
		    previous_delta_rotation = 0.0;
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
		app_manager->getCurrentTimer ()->setTimeLeft (new_time);
	    }
	}
    }
}
void AnalogTimer::paintEvent (QPaintEvent*)
{
    QPainter p (this);
    p.setRenderHint (QPainter::Antialiasing, true);
    p.setRenderHint (QPainter::SmoothPixmapTransform, true);
    p.setPen (Qt::NoPen);
    if (KITCHENTIMER_SHOW_DEBUG_OVERLAY) {
	p.setBrush (QColor (0xff, 0, 0, 60));
	p.drawRect (rect ());
    }

    switch (KITCHENTIMER_ANALOG_TIMER_MODE) {
    case 0: {
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
		    p.setTransform (tr);
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
		p.setTransform (tr);
		p.drawRect (QRectF (estimated_circle_center + QPointF (-estimated_circle_rect.width ()*0.005, estimated_circle_rect.height ()*0.29),
				    QSizeF (estimated_circle_rect.width ()*0.01, estimated_circle_rect.height ()*0.045)));
	    }
	}

	{
	    QTransform tr;
	    tr.translate (estimated_circle_center.x (), estimated_circle_center.y ());
	    tr.rotate (angle);
	    tr.translate (-estimated_circle_center.x (), -estimated_circle_center.y ());
	    p.setTransform (tr);
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
	    p.setBrush (QColor (0xc0, 0x00, 0x00));
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
	int full_hours = int_value/3600;
	int int_angle = int_value%3600;
	int_angle = ((int_angle + 59)/60)*60;
	double angle = double (int_angle)*0.1;

	bool draw_down = down && !edit_mode.isHalted ();
	double press_scale_factor = draw_down ? 0.97 : 1.0;

	QColor core_color = draw_down ? QColor (0xe8, 0xe8, 0xe8) : QColor (0xfd, 0xfd, 0xfd);

	p.drawImage (estimated_circle_rect, resource_manager->analog_timer_core_image, resource_manager->analog_timer_core_image.rect ());
	p.save ();
	{
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
	    QRadialGradient current_grad (estimated_circle_center, estimated_circle_radius*0.89);
	    current_grad.setColorAt (0.0, transparent_color);
	    current_grad.setColorAt (start, transparent_color);
	    current_grad.setColorAt (1.0, transparent_color);
	    QRadialGradient current2_grad (estimated_circle_center, estimated_circle_radius*0.89);
	    current2_grad.setColorAt (0.0, transparent_color);
	    current2_grad.setColorAt (start, transparent_color);
	    current2_grad.setColorAt (1.0, transparent_color);
	    QRadialGradient current3_grad (estimated_circle_center, estimated_circle_radius*0.89);
	    current3_grad.setColorAt (0.0, transparent_color);
	    current3_grad.setColorAt (start, transparent_color);
	    current3_grad.setColorAt (1.0, transparent_color);
	    QRadialGradient full_grad (estimated_circle_center, estimated_circle_radius*0.89);
	    full_grad.setColorAt (0.0, transparent_color);
	    full_grad.setColorAt (start, transparent_color);
	    full_grad.setColorAt (1.0, transparent_color);
	    double off = 0.0;
	    current_grad.setColorAt (start + range_f*(off += 1.0)/range_l, current_color);
	    current2_grad.setColorAt (start + range_f*(off)/range_l, current2_color);
	    current3_grad.setColorAt (start + range_f*(off)/range_l, current3_color);
	    current_grad.setColorAt (start + range_f*(off += current_l)/range_l, current_color);
	    current2_grad.setColorAt (start + range_f*(off)/range_l, current2_color);
	    current3_grad.setColorAt (start + range_f*(off)/range_l, current3_color);
	    current_grad.setColorAt (start + range_f*(off += 1.0)/range_l, transparent_color);
	    current2_grad.setColorAt (start + range_f*(off)/range_l, transparent_color);
	    current3_grad.setColorAt (start + range_f*(off)/range_l, transparent_color);
	    full_grad.setColorAt (start + range_f*off/range_l, transparent_color);
	    for (int i = 0; i < full_hours; ++i) {
		full_grad.setColorAt (start + range_f*(off += tr_l)/range_l, transparent_color);
		full_grad.setColorAt (start + range_f*(off += 1.0)/range_l, full_color);
		full_grad.setColorAt (start + range_f*(off += full_l)/range_l, full_color);
		full_grad.setColorAt (start + range_f*(off += 1.0)/range_l, transparent_color);
	    }
	    p.setBrush (full_grad);
	    p.drawPie (getScaledRect (estimated_circle_rect, 0.89), 0, 5760);
	    p.setBrush (current_grad);
	    if (!edit_mode.isEnabled ()) {
		if (int_angle) {
		    p.drawPie (getScaledRect (estimated_circle_rect, 0.89), 5760/4, (6 - angle)/360*5760);
		    if (lifetime_elapsed_timer.elapsed ()%500 < 250) {
			p.setBrush (current2_grad);
		    } else {
			p.setBrush (current3_grad);
		    }
		    p.drawPie (getScaledRect (estimated_circle_rect, 0.89), 5760/4 + (6.0 - angle)/360*5760, (-6.0)/360*5760);
		}
	    } else {
		p.drawPie (getScaledRect (estimated_circle_rect, 0.89), 5760/4, -angle/360*5760);
	    }
	}
	p.restore ();
	{
	    QTransform tr;
	    tr.translate (estimated_circle_center.x (), estimated_circle_center.y ());
	    tr.rotate (angle - 90);
	    tr.translate (-estimated_circle_center.x (), -estimated_circle_center.y ());
	    p.setTransform (tr);
	    p.drawImage (estimated_circle_handle_rect, resource_manager->analog_timer_handle_image,
			 resource_manager->analog_timer_handle_image.rect ());
	    p.resetTransform ();
	}

	{
	    p.setBrush (QColor (0xd7, 0xb1, 0xa0));
	    double angle = 0.0;
	    for (int i = 0; i < 12*5; ++i, angle += 6.0) {
		if (i%5) {
		    QTransform tr;
		    tr.translate (estimated_circle_center.x (), estimated_circle_center.y ());
		    tr.rotate (angle);
		    tr.translate (-estimated_circle_center.x (), -estimated_circle_center.y ());
		    p.setTransform (tr);
		    p.drawRect (QRectF (estimated_circle_center + QPointF (-estimated_circle_rect.width ()*0.003,
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
		tr.translate (estimated_circle_center.x (), estimated_circle_center.y ());
		tr.rotate (angle);
		tr.translate (-estimated_circle_center.x (), -estimated_circle_center.y ());
		p.setTransform (tr);
		p.drawRect (QRectF (estimated_circle_center + QPointF (-estimated_circle_rect.width ()*0.005, estimated_circle_rect.height ()*0.29),
				    QSizeF (estimated_circle_rect.width ()*0.01, estimated_circle_rect.height ()*((i%3) ? 0.035 : 0.05))));
	    }
	}

	p.resetTransform ();
	angle = M_PI*0.5*9.0/3.0;
	int n = 0;
	for (int i = 0; i < 12; ++i) {
	    double text_radius = estimated_circle_radius*((i%3) ? 0.75 : 0.8);
	    // p.setPen (QColor (0x00, 0, 0, 0xcc));
	    // p.drawText (QRectF (QPointF (estimated_circle_center - QPointF (estimated_font_pixel_size*1.5 - text_radius*cos (angle) - 0.8,
	    // 								    estimated_font_pixel_size*1.5 - text_radius*sin (angle) - 0.5)),
	    // 			QSizeF (estimated_font_pixel_size*3.0, estimated_font_pixel_size*3.0)), Qt::AlignCenter, QString::number (n));
	    p.setPen (QColor (0xc0, 0, 0));
	    p.drawText (QRectF (QPointF (estimated_circle_center - QPointF (estimated_font_pixel_size*1.5 - text_radius*cos (angle),
									    estimated_font_pixel_size*1.5 - text_radius*sin (angle))),
				QSizeF (estimated_font_pixel_size*3.0, estimated_font_pixel_size*3.0)), Qt::AlignCenter, QString::number (n));
	    angle += M_PI*0.5*1.0/3.0;
	    n += 5;
	}
    } break;
    case 2: {
	int int_value = QTime (0, 0, 0).secsTo (app_manager->getCurrentTimer ()->getTimeLeft ());
	int full_hours = int_value/3600;
	int int_angle = int_value%3600;
	int_angle = ((int_angle + 59)/60)*60;
	double angle = double (int_angle)*0.1;

	bool draw_down = down && !edit_mode.isHalted ();
	double press_scale_factor = draw_down ? 0.97 : 1.0;

	QColor core_color = draw_down ? QColor (0xe8, 0xe8, 0xe8) : QColor (0xfd, 0xfd, 0xfd);

	p.drawImage (estimated_circle_rect, resource_manager->analog_timer_core_image, resource_manager->analog_timer_core_image.rect ());
	p.save ();
	{
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
	    QRadialGradient current_grad (estimated_circle_center, estimated_circle_radius*0.89);
	    current_grad.setColorAt (0.0, transparent_color);
	    current_grad.setColorAt (start, transparent_color);
	    current_grad.setColorAt (1.0, transparent_color);
	    QRadialGradient current2_grad (estimated_circle_center, estimated_circle_radius*0.89);
	    current2_grad.setColorAt (0.0, transparent_color);
	    current2_grad.setColorAt (start, transparent_color);
	    current2_grad.setColorAt (1.0, transparent_color);
	    QRadialGradient current3_grad (estimated_circle_center, estimated_circle_radius*0.89);
	    current3_grad.setColorAt (0.0, transparent_color);
	    current3_grad.setColorAt (start, transparent_color);
	    current3_grad.setColorAt (1.0, transparent_color);
	    QRadialGradient full_grad (estimated_circle_center, estimated_circle_radius*0.89);
	    full_grad.setColorAt (0.0, transparent_color);
	    full_grad.setColorAt (start, transparent_color);
	    full_grad.setColorAt (1.0, transparent_color);
	    double off = 0.0;
	    current_grad.setColorAt (start + range_f*(off += 1.0)/range_l, current_color);
	    current2_grad.setColorAt (start + range_f*(off)/range_l, current2_color);
	    current3_grad.setColorAt (start + range_f*(off)/range_l, current3_color);
	    current_grad.setColorAt (start + range_f*(off += current_l)/range_l, current_color);
	    current2_grad.setColorAt (start + range_f*(off)/range_l, current2_color);
	    current3_grad.setColorAt (start + range_f*(off)/range_l, current3_color);
	    current_grad.setColorAt (start + range_f*(off += 1.0)/range_l, transparent_color);
	    current2_grad.setColorAt (start + range_f*(off)/range_l, transparent_color);
	    current3_grad.setColorAt (start + range_f*(off)/range_l, transparent_color);
	    full_grad.setColorAt (start + range_f*off/range_l, transparent_color);
	    for (int i = 0; i < full_hours; ++i) {
		full_grad.setColorAt (start + range_f*(off += tr_l)/range_l, transparent_color);
		full_grad.setColorAt (start + range_f*(off += 1.0)/range_l, full_color);
		full_grad.setColorAt (start + range_f*(off += full_l)/range_l, full_color);
		full_grad.setColorAt (start + range_f*(off += 1.0)/range_l, transparent_color);
	    }
	    p.setBrush (full_grad);
	    p.drawPie (getScaledRect (estimated_circle_rect, 0.89), 0, 5760);
	    p.setBrush (current_grad);
	    if (app_manager->getCurrentTimer ()->isRunning ()) {
		if (int_angle) {
		    p.drawPie (getScaledRect (estimated_circle_rect, 0.89), 5760/4, (6 - angle)/360*5760);
		    if (lifetime_elapsed_timer.elapsed ()%500 < 250) {
			p.setBrush (current2_grad);
		    } else {
			p.setBrush (current3_grad);
		    }
		    p.drawPie (getScaledRect (estimated_circle_rect, 0.89), 5760/4 + (6.0 - angle)/360*5760, (-6.0)/360*5760);
		}
	    } else {
		p.drawPie (getScaledRect (estimated_circle_rect, 0.89), 5760/4, -angle/360*5760);
	    }
	}
	p.restore ();
	{
	    QTransform tr;
	    tr.translate (estimated_circle_center.x (), estimated_circle_center.y ());
	    tr.rotate (angle - 90);
	    tr.translate (-estimated_circle_center.x (), -estimated_circle_center.y ());
	    p.setTransform (tr);
	    p.drawImage (estimated_circle_handle_rect, resource_manager->analog_timer_handle_image,
			 resource_manager->analog_timer_handle_image.rect ());
	    p.resetTransform ();
	}

	{
	    p.setBrush (QColor (0xa8, 0x65, 0x2b));
	    double angle = 0.0;
	    for (int i = 0; i < 12*5; ++i, angle += 6.0) {
		if (i%5) {
		    QTransform tr;
		    tr.translate (estimated_circle_center.x (), estimated_circle_center.y ());
		    tr.rotate (angle);
		    tr.translate (-estimated_circle_center.x (), -estimated_circle_center.y ());
		    p.setTransform (tr);
		    p.drawRect (QRectF (estimated_circle_center + QPointF (-estimated_circle_rect.width ()*0.003,
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
		tr.translate (estimated_circle_center.x (), estimated_circle_center.y ());
		tr.rotate (angle);
		tr.translate (-estimated_circle_center.x (), -estimated_circle_center.y ());
		p.setTransform (tr);
		p.drawRect (QRectF (estimated_circle_center + QPointF (-estimated_circle_rect.width ()*0.005, estimated_circle_rect.height ()*0.29),
				    QSizeF (estimated_circle_rect.width ()*0.01, estimated_circle_rect.height ()*((i%3) ? 0.035 : 0.06))));
	    }
	}

	p.resetTransform ();
	angle = M_PI*0.5*9.0/3.0;
	int n = 0;
	for (int i = 0; i < 12; ++i) {
	    double text_radius = estimated_circle_radius*((i%3) ? 0.75 : 0.8);
	    if (i%3)
		p.setFont (small_font);
	    else
		p.setFont (font);
	    // p.setPen (QColor (0x00, 0, 0, 0xcc));
	    // p.drawText (QRectF (QPointF (estimated_circle_center - QPointF (estimated_font_pixel_size*1.5 - text_radius*cos (angle) - 0.8,
	    // 								    estimated_font_pixel_size*1.5 - text_radius*sin (angle) - 0.5)),
	    // 			QSizeF (estimated_font_pixel_size*3.0, estimated_font_pixel_size*3.0)), Qt::AlignCenter, QString::number (n));
	    p.setPen (QColor (0xc0, 0, 0));
	    p.drawText (QRectF (QPointF (estimated_circle_center - QPointF (estimated_font_pixel_size*1.5 - text_radius*cos (angle),
									    estimated_font_pixel_size*1.5 - text_radius*sin (angle))),
				QSizeF (estimated_font_pixel_size*3.0, estimated_font_pixel_size*3.0)), Qt::AlignCenter, QString::number (n));
	    angle += M_PI*0.5*1.0/3.0;
	    n += 5;
	}
    } break;
    }
}
