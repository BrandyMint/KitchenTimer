#include <QPainter>
#include <QMouseEvent>
#include <QtCore/qmath.h>
#include <QApplication>

#include "analogtimer.h"

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

AnalogTimer::AnalogTimer (QWidget *parent)
    : QWidget (parent), edit_mode (false), time_value (0, 0, 0), pressed (false)
{
    setMinimumSize (QSize (60, 60));
}
AnalogTimer::~AnalogTimer ()
{
}
void AnalogTimer::setEditMode (bool new_edit_mode)
{
    if (edit_mode != new_edit_mode)
	edit_mode = new_edit_mode;
}
QTime AnalogTimer::getTime ()
{
    return time_value;
}
void AnalogTimer::setTime (const QTime &new_time)
{
    time_value = new_time;
    if (time_value > MAX_TIME) {
	time_value = MAX_TIME;
    }	
    emit timeChanged (time_value);
    update ();
}
bool AnalogTimer::isSliderDown ()
{
    return pressed;
}
void AnalogTimer::getValuableGeometry (QPoint &start_point, QSize &circle_size)
{
    start_point = QPoint (0, 0);
    circle_size = size ();
    if (circle_size.width () > circle_size.height ()) {
	start_point.setX ((circle_size.width () - circle_size.height ()) >> 1);
	circle_size.setWidth (circle_size.height ());
    }
    if (circle_size.height () > circle_size.width ()) {
	start_point.setY ((circle_size.height () - circle_size.width ()) >> 1);
	circle_size.setHeight (circle_size.width ());
    }
}
double AnalogTimer::getRotationByPos (const QPoint &point_pos, const QSize &circle_size)
{
    QPoint rel_pos = point_pos - QPoint (circle_size.width () >> 1, circle_size.height () >> 1);
    return 0.5 - qAtan2 (rel_pos.x (), rel_pos.y ())*0.1591549430919;
}
void AnalogTimer::mouseMoveEvent (QMouseEvent *event)
{
    emit clearAlarms ();
    if (pressed) {
	if (edit_mode) {
	    QPoint start_point;
	    QSize circle_size;
	    getValuableGeometry (start_point, circle_size);
	    QPoint current_pos = event->pos () - start_point;
	    
	    QPointF circle_center (start_point.x () + circle_size.width ()*0.5, start_point.y () + circle_size.height ()*0.5);
	    double circle_radius = circle_size.width ()*0.05;
	    QPointF tmp = circle_center - event->pos ();
	    if (QPointF::dotProduct (tmp, tmp) < circle_radius*circle_radius) {
		halted = true;
	    } else if (halted) {
		pressed_time = time_value;
		pressed_local_rotation = getRotationByPos (current_pos, circle_size);
		previous_delta_rotation = 0.0;
		halted = false;
	    } else {
		double local_rotation = getRotationByPos (current_pos, circle_size);
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
		setTime (new_time);
	    }
	}
    }
}
void AnalogTimer::mousePressEvent (QMouseEvent *event)
{
    QPoint start_point;
    QSize circle_size;
    getValuableGeometry (start_point, circle_size);
    QPointF circle_center (start_point.x () + circle_size.width ()*0.5, start_point.y () + circle_size.height ()*0.5);
    double circle_radius = circle_size.width ()*0.5;
    QPointF tmp = circle_center - event->pos ();
    if (QPointF::dotProduct (tmp, tmp) > circle_radius*circle_radius) {
	event->ignore ();
	return;
    }
    emit clearAlarms ();
    if (event->button () == Qt::LeftButton) {
	if (edit_mode) {
	    QPoint start_point;
	    QSize circle_size;
	    getValuableGeometry (start_point, circle_size);
	    QPoint current_pos = event->pos () - start_point;
	    pressed = true;
	    pressed_time = time_value;
	    pressed_local_rotation = getRotationByPos (current_pos, circle_size);
	    previous_delta_rotation = 0.0;
	    halted = false;
	    update ();
	} else {
	    emit enterEditModeRequested ();
	}
    }
}
void AnalogTimer::mouseReleaseEvent (QMouseEvent *event)
{
    QPoint start_point;
    QSize circle_size;
    getValuableGeometry (start_point, circle_size);
    QPointF circle_center (start_point.x () + circle_size.width ()*0.5, start_point.y () + circle_size.height ()*0.5);
    double circle_radius = circle_size.width ()*0.5;
    QPointF tmp = circle_center - event->pos ();
    if (QPointF::dotProduct (tmp, tmp) > circle_radius*circle_radius) {
	event->ignore ();
	return;
    }
    emit clearAlarms ();
    if (event->button () == Qt::LeftButton) {
	if (pressed) {
	    pressed = false;
	    if (edit_mode)
		emit leaveEditModeRequested ();
	    update ();
	}
    }
}
void AnalogTimer::paintEvent (QPaintEvent *)
{
    QPoint start_point;
    QSize circle_size;
    getValuableGeometry (start_point, circle_size);
    QRectF circle_rect (start_point, circle_size);
    QPointF circle_center (start_point.x () + circle_size.width ()*0.5, start_point.y () + circle_size.height ()*0.5);
    double circle_radius = circle_rect.width ()*0.5;

    int int_value = QTime (0, 0, 0).secsTo (getTime ());
    int full_hours = int_value/3600;
    int int_angle = int_value%3600;
    int_angle = ((int_angle + 30)/60)*60;
    double angle = double (int_angle)*0.1;

    QPainter p (this);
    p.setRenderHint (QPainter::Antialiasing, true);
    p.setPen (Qt::NoPen);

    double press_scale_factor = pressed ? 0.97 : 1.0;

    QColor core_color = pressed ? QColor (0xe8, 0xe8, 0xe8) : QColor (0xfd, 0xfd, 0xfd);

    {
	p.setBrush (QColor (0, 0, 0, 80));
	p.drawPie (circle_rect, 0, 5760);
	p.setBrush (QColor (0xf3, 0xf3, 0xf3));
	p.drawPie (getScaledRect (circle_rect, 0.96899224806202), 0, 5760);
	p.setBrush (QColor (0xff, 0xff, 0xff));
	p.drawPie (getScaledRect (circle_rect, 0.89147286821705), 0, 5760);
	p.setBrush (QColor (0xc0, 0x33, 0x33));
	p.drawPie (getScaledRect (circle_rect, 0.6), 1440, -(int_angle << 3)/5);
	p.setBrush (QColor (0xcc, 0xcc, 0xcc));
	p.drawPie (getScaledRect (circle_rect, 0.55813953488372), 0, 5760);
	if (pressed) {
	    p.setBrush (QColor (0xc3, 0xc3, 0xc3));
	    p.drawPie (getScaledRect (circle_rect, 0.53488372093023), 0, 5760);
	}
	p.setBrush (core_color);
	p.drawPie (getScaledRect (circle_rect, 0.53488372093023*press_scale_factor), 0, 5760);
    }

    {
	p.setBrush (QColor (180, 180, 180));
	double angle = 0.0;
	for (int i = 0; i < 12*5; ++i, angle += 6.0) {
	    if (i%5) {
		QTransform tr;
		tr.translate (circle_center.x (), circle_center.y ());
		tr.rotate (angle);
		tr.translate (-circle_center.x (), -circle_center.y ());
		p.setTransform (tr);
		p.drawRect (QRectF (circle_center + QPointF (-circle_rect.width ()*0.003, circle_rect.height ()*0.29),
				    QSizeF (circle_rect.width ()*0.006, circle_rect.height ()*0.035)));
	    }
	}
    }

    {
	p.setBrush (QColor (20, 20, 20));
	double angle = 0.0;
	for (int i = 0; i < 12; ++i, angle += 30.0) {
	    QTransform tr;
	    tr.translate (circle_center.x (), circle_center.y ());
	    tr.rotate (angle);
	    tr.translate (-circle_center.x (), -circle_center.y ());
	    p.setTransform (tr);
	    p.drawRect (QRectF (circle_center + QPointF (-circle_rect.width ()*0.005, circle_rect.height ()*0.29),
				QSizeF (circle_rect.width ()*0.01, circle_rect.height ()*0.045)));
	}
    }

    {
	QTransform tr;
	tr.translate (circle_center.x (), circle_center.y ());
	tr.rotate (angle);
	tr.translate (-circle_center.x (), -circle_center.y ());
	p.setTransform (tr);
	QRectF handle_shadow_rect = getScaledRect (circle_rect, 0.137519379844961*press_scale_factor, 0.51162790697674*press_scale_factor);
	QLinearGradient handle_shadow_grad (handle_shadow_rect.topLeft () + QPointF (0.0, handle_shadow_rect.height ()*0.5),
					    handle_shadow_rect.topLeft () + QPointF (handle_shadow_rect.width (), handle_shadow_rect.height ()*0.5));
	handle_shadow_grad.setColorAt (0, core_color);
	handle_shadow_grad.setColorAt (0.25, QColor (0xc0, 0xc0, 0xc0));
	handle_shadow_grad.setColorAt (0.75, QColor (0xc0, 0xc0, 0xc0));
	handle_shadow_grad.setColorAt (1, core_color);
	p.setBrush (handle_shadow_grad);
	p.drawRect (handle_shadow_rect);
	QRectF handle_rect = getScaledRect (circle_rect, 0.077519379844961*press_scale_factor, 0.51162790697674*press_scale_factor);
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
    double radius = circle_radius*0.8;
    QFont small_font = qApp->font ();
    double font_pixel_size = circle_rect.width ()*0.1;
    small_font.setPixelSize (font_pixel_size);
    p.setFont (small_font);
    p.setPen (QColor (0xc0, 0, 0));
    angle = M_PI*0.5*9.0/3.0;
    int n = 0;
    for (int i = 0; i < 12; ++i) {
	p.drawText (QRectF (QPointF (circle_center - QPointF (font_pixel_size*1.5 - radius*cos (angle), font_pixel_size*1.5 - radius*sin (angle))),
			    QSizeF (font_pixel_size*3.0, font_pixel_size*3.0)), Qt::AlignCenter, QString::number (n));
	angle += M_PI*0.5*1.0/3.0;
	n += 5;
    }
}
