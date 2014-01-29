#include <QPainter>
#include <QMouseEvent>
#include <QtCore/qmath.h>

#include "clickablelabel.h"
#include "application.h"

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
    // if (event->button () == Qt::LeftButton) {
    // 	QPoint start_point;
    // 	QSize circle_size;
    // 	getValuableGeometry (start_point, circle_size);
    // 	QPoint current_pos = event->pos () - start_point;
    // 	setValueByPos (current_pos, circle_size);
    // 	setSliderDown (true);
    // }
}
void ClickableLabel::mouseReleaseEvent (QMouseEvent *event)
{
    // if (event->button () == Qt::LeftButton) {
    // 	setSliderDown (false);
    // }
}
