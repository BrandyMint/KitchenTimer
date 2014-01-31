#include <QPainter>
#include <QMouseEvent>
#include <QtCore/qmath.h>
#include <QApplication>

#include "digitaltimer.h"

DigitalTimer::DigitalTimer (QWidget *parent)
    : QLabel (parent), value (0, 0, 0), edit_mode (false), font (qApp->font ())
{
    font.setPointSize (26);
    setFont (font);
    setStyleSheet ("QLabel { color : #fffffd; }");
    setAlignment (Qt::AlignHCenter | Qt::AlignVCenter);
}
DigitalTimer::~DigitalTimer ()
{
}
void DigitalTimer::setTime (const QTime &new_value)
{
    value = new_value;
    setText (value.toString ());
}
void DigitalTimer::setEditMode (bool new_edit_mode)
{
    edit_mode = new_edit_mode;
}
void DigitalTimer::mouseMoveEvent (QMouseEvent*)
{
}
void DigitalTimer::mousePressEvent (QMouseEvent *event)
{
    if (event->button () == Qt::LeftButton) {
	if (!edit_mode)
	    emit enterEditModeRequested ();
    }
}
void DigitalTimer::mouseReleaseEvent (QMouseEvent *event)
{
    if (event->button () == Qt::LeftButton) {
    }
}
