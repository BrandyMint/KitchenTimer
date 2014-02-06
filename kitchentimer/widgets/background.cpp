#include "background.h"
#include "resourcemanager.h"

#include <QPainter>
#include <QMouseEvent>


Background::Background (QWidget *parent)
    : QWidget (parent), transition_timeout (0), shaded (false)
{
    elapsed_timer.invalidate ();
    repaint_timer.setSingleShot (true);
    connect (&repaint_timer, SIGNAL (timeout ()), this, SLOT (checkUpdate ()));
}
void Background::mousePressEvent (QMouseEvent *event)
{
    if (event->button () == Qt::LeftButton) {
	emit pressed ();
    }
}
void Background::mouseReleaseEvent (QMouseEvent *event)
{
    if (event->button () == Qt::LeftButton) {
	emit released ();
    }
}
void Background::paintEvent (QPaintEvent*)
{
    QPainter p (this);
    p.setRenderHint (QPainter::SmoothPixmapTransform, true);
    QRect src_rect = resource_manager->background_image.rect ();
    const QRect &dst_rect = rect ();
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
    
    p.drawImage (dst_rect, resource_manager->background_image, src_rect);
    if (shaded) {
	p.setPen (Qt::NoPen);
	if (elapsed_timer.isValid ()) {
	    int elapsed = elapsed_timer.elapsed ();
	    if (elapsed >= transition_timeout) {
		elapsed_timer.invalidate ();
		p.setBrush (QColor (0, 0, 0, 192));
	    } else {
		double m = double (elapsed)/transition_timeout;
		p.setBrush (QColor (0, 0, 0, int (m*192)));
	    }
	} else {
	    p.setBrush (QColor (0, 0, 0, 192));
	}
	p.drawRect (dst_rect);
    } else {
	if (elapsed_timer.isValid ()) {
	    int elapsed = elapsed_timer.elapsed ();
	    if (elapsed >= transition_timeout) {
		elapsed_timer.invalidate ();
	    } else {
		double m = 1.0 - double (elapsed)/transition_timeout;
		p.setPen (Qt::NoPen);
		p.setBrush (QColor (0, 0, 0, int (m*192)));
		p.drawRect (dst_rect);
	    }
	}
    }
}
void Background::setShaded (bool new_shaded)
{
    shaded = new_shaded;
    elapsed_timer.invalidate ();
    update ();
}
void Background::startShading (int new_transition_timeout)
{
    transition_timeout = new_transition_timeout;
    elapsed_timer.start ();
    repaint_timer.start (KITCHENTIMER_ANIMATION_REPAINT_TIMEOUT_MS);
    shaded = true;
    update ();
}
void Background::startUnshading (int new_transition_timeout)
{
    transition_timeout = new_transition_timeout;
    elapsed_timer.start ();
    shaded = false;
    update ();
}
void Background::checkUpdate ()
{
    repaint ();
    if (elapsed_timer.isValid () && (elapsed_timer.elapsed () < transition_timeout)) {
	repaint_timer.start (KITCHENTIMER_ANIMATION_REPAINT_TIMEOUT_MS);
    } else {
	elapsed_timer.invalidate ();
	if (shaded)
	    emit shadingDone ();
	else
	    emit unshadingDone ();
	update ();
    }
}
