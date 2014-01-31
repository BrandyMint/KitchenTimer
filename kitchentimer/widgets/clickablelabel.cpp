#include <QPainter>
#include <QMouseEvent>
#include <QtCore/qmath.h>

#include "clickablelabel.h"

ClickableLabel::ClickableLabel (QWidget *parent)
    : QLabel (parent)
{
}
ClickableLabel::~ClickableLabel ()
{
}
void ClickableLabel::mouseMoveEvent (QMouseEvent *event)
{
    // if (isSliderDown ()) {
    // 	QPoint start_point;
    // 	QSize circle_size;
    // 	getValuableGeometry (start_point, circle_size);
    // 	QPoint current_pos = event->pos () - start_point;
    // 	setValueByPos (current_pos, circle_size);
    // }
}
void ClickableLabel::mousePressEvent (QMouseEvent *event)
{
    if (event->button () == Qt::LeftButton) {
	emit pressed ();
    }
}
void ClickableLabel::mouseReleaseEvent (QMouseEvent *event)
{
    if (event->button () == Qt::LeftButton) {
	emit released ();
	emit clicked ();
    }
}
