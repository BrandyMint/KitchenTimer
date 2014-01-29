#include <QPainter>
#include <QMouseEvent>
#include <QtCore/qmath.h>

#include "customdial.h"

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

CustomDial::CustomDial (QWidget *parent)
    : QAbstractSlider (parent)
{
    setRange (0, 60*55 - 1);
    setMinimumSize (QSize (60, 60));
}
CustomDial::~CustomDial ()
{
}
void CustomDial::getValuableGeometry (QPoint &start_point, QSize &circle_size)
{
    start_point = QPoint (0, 0);
    circle_size = size ();
    if (circle_size.width () > circle_size.height ()) {
	int delta = circle_size.width () - circle_size.height ();
	start_point.setX (delta >> 1);
	circle_size.setWidth (circle_size.height ());
    }
    if (circle_size.height () > circle_size.width ()) {
	int delta = circle_size.height () - circle_size.width ();
	start_point.setY (delta >> 1);
	circle_size.setHeight (circle_size.width ());
    }
}
void CustomDial::setValueByPos (const QPoint &point_pos, const QSize &circle_size)
{
    QPoint rel_pos = point_pos - QPoint (circle_size.width () >> 1, circle_size.height () >> 1);
    double value = qAtan2 (rel_pos.x (), -rel_pos.y ())*0.31830988618377;
    if (value < 0.0) {
	value = (value > (-5.0/30.0)) ? 0.0 : (2.0 + value);
    } else if (value > 1.0) {
	value = 1.0;
    }
    value *= 30.0/55.0;
    int range = abs (maximum () - minimum ());
    int int_value = value*range;
    setValue (int_value);
}
void CustomDial::paintEvent (QPaintEvent *)
{
    QPoint start_point;
    QSize circle_size;
    getValuableGeometry (start_point, circle_size);
    QRectF circle_rect (start_point, circle_size);
    QPointF circle_center (start_point.x () + circle_size.width ()*0.5, start_point.y () + circle_size.height ()*0.5);

    int int_value = value () - minimum ();
    int range = abs (maximum () - minimum ());
    double value = double (int_value)/range;
    double angle = value*330;

    QPainter p (this);
    p.setRenderHint (QPainter::Antialiasing, true);
    p.setPen (Qt::NoPen);

    {
	p.setBrush (QColor (0, 0, 0, 80));
	p.drawPie (circle_rect, 0, 5760);
	p.setBrush (QColor (0xf3, 0xf3, 0xf3));
	p.drawPie (getScaledRect (circle_rect, 0.96899224806202), 0, 5760);
	p.setBrush (QColor (0xff, 0xff, 0xff));
	p.drawPie (getScaledRect (circle_rect, 0.89147286821705), 0, 5760);
	p.setBrush (QColor (0xcc, 0xcc, 0xcc));
	p.drawPie (getScaledRect (circle_rect, 0.55813953488372), 0, 5760);
	p.setBrush (QColor (0xfd, 0xfd, 0xfd));
	p.drawPie (getScaledRect (circle_rect, 0.53488372093023), 0, 5760);
	p.setBrush (QColor (0xff, 0, 0));
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
	QRectF handle_shadow_rect = getScaledRect (circle_rect, 0.137519379844961, 0.51162790697674);
	QLinearGradient handle_shadow_grad (handle_shadow_rect.topLeft () + QPointF (0.0, handle_shadow_rect.height ()*0.5),
					    handle_shadow_rect.topLeft () + QPointF (handle_shadow_rect.width (), handle_shadow_rect.height ()*0.5));
	handle_shadow_grad.setColorAt (0, Qt::white);
	handle_shadow_grad.setColorAt (0.25, QColor (0xe6, 0xe6, 0xe6));
	handle_shadow_grad.setColorAt (0.75, QColor (0xe6, 0xe6, 0xe6));
	handle_shadow_grad.setColorAt (1, Qt::white);
	p.setBrush (handle_shadow_grad);
	p.drawRect (handle_shadow_rect);
	QRectF handle_rect = getScaledRect (circle_rect, 0.077519379844961, 0.51162790697674);
	QLinearGradient handle_grad (handle_rect.topLeft () + QPointF (handle_rect.width ()*0.5, 0.0),
				     handle_rect.topLeft () + QPointF (handle_rect.width ()*0.5, handle_rect.height ()));
	handle_grad.setColorAt (0, Qt::white);
	handle_grad.setColorAt (0.20, QColor (0xe6, 0xe6, 0xe6));
	handle_grad.setColorAt (0.45, Qt::white);
	handle_grad.setColorAt (0.55, Qt::white);
	handle_grad.setColorAt (0.75, QColor (0xe6, 0xe6, 0xe6));
	handle_grad.setColorAt (1, Qt::white);
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
    double radius = circle_rect.width ()*0.4;
    QFont small_font = QFont ("CartonsixNC");
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
void CustomDial::mouseMoveEvent (QMouseEvent *event)
{
    if (isSliderDown ()) {
	QPoint start_point;
	QSize circle_size;
	getValuableGeometry (start_point, circle_size);
	QPoint current_pos = event->pos () - start_point;
	setValueByPos (current_pos, circle_size);
    }
}
void CustomDial::mousePressEvent (QMouseEvent *event)
{
    if (event->button () == Qt::LeftButton) {
	QPoint start_point;
	QSize circle_size;
	getValuableGeometry (start_point, circle_size);
	QPoint current_pos = event->pos () - start_point;
	setValueByPos (current_pos, circle_size);
	setSliderDown (true);
    }
}
void CustomDial::mouseReleaseEvent (QMouseEvent *event)
{
    if (event->button () == Qt::LeftButton) {
	setSliderDown (false);
    }
}
