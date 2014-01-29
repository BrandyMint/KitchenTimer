#include "background.h"
#include "application.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>
#include <QPainter>


Background::Background (QWidget *parent)
    : QWidget (parent)
{
}
void Background::paintEvent (QPaintEvent*)
{
    QPainter p (this);
    p.setRenderHint (QPainter::SmoothPixmapTransform, true);
    QRect src_rect = app->background_image.rect ();
    QRect dst_rect = rect ();
    int src_w = src_rect.width ();
    int src_h = src_rect.height ();
    int dst_w = dst_rect.width ();
    int dst_h = dst_rect.height ();
    if (src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0)
	return;
    double src_aspect = double (src_w)/double (src_h);
    double dst_aspect = double (dst_w)/double (dst_h);
    if (src_aspect > dst_aspect) {
	src_rect.setX ((src_w - int (src_h*dst_aspect)) >> 1);
	src_rect.setWidth (src_h*dst_aspect);
    } else if (src_aspect < dst_aspect) {
	src_rect.setY ((src_h - int (src_w/dst_aspect)) >> 1);
	src_rect.setHeight (src_w/dst_aspect);
    }
    
    p.drawImage (dst_rect, app->background_image, src_rect);
}
