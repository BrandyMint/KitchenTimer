#include "buttonstick.h"
#include "resourcemanager.h"

#include <QPainter>
#include <QMouseEvent>
#include <QPicture>
#include <QSvgRenderer>


#define MAX_WIDTH_FRACTION 0.15
#define MAX_HEIGHT_FRACTION 0.15
#define OFFSET_HEIGHT_FRACTION 0.5


ButtonStick::ButtonStick (QWidget *parent)
    : QWidget (parent), audio_enabled (true), audio_pressed (false), mouse_inside (false)
{
    long_press_timer.setSingleShot (true);
    connect (&long_press_timer, SIGNAL (timeout ()), this, SLOT (privateLongPressed ()));
}
ButtonStick::~ButtonStick ()
{
}
void ButtonStick::adjustGeometry (const QRect &parent_rect)
{
    const QSize &image_size = resource_manager->button_stick_image.size ();
    int image_h = image_size.height ();
    if (image_h <= 0) return;
    double image_w = image_size.width ();
    double stick_w = parent_rect.width ()*MAX_WIDTH_FRACTION;
    double stick_h = stick_w*image_w/double (image_h);
    double max_stick_h = parent_rect.height ()*MAX_HEIGHT_FRACTION;
    if (stick_h > max_stick_h) {
    	stick_w = max_stick_h*stick_w/stick_h;
    	stick_h = max_stick_h;
    }
    setGeometry (0, int (stick_h*OFFSET_HEIGHT_FRACTION), int (stick_w), int (stick_h));
}
bool ButtonStick::getAudioEnabled ()
{
    return audio_enabled;
}
void ButtonStick::setAudioEnabled (bool new_audio_enabled)
{
    audio_enabled = new_audio_enabled;
    update ();
}
void ButtonStick::resizeEvent (QResizeEvent*)
{
    cached_image = QImage (size (), QImage::Format_ARGB32);
    cached_image.fill (0);

    QPainter p;
    p.begin (&cached_image);
    p.setRenderHint (QPainter::SmoothPixmapTransform, true);
    p.drawImage (rect (), resource_manager->button_stick_image, resource_manager->button_stick_image.rect ());
    p.end ();
}
void ButtonStick::mousePressEvent (QMouseEvent *event)
{
    if (event->button () == Qt::LeftButton) {
	audio_pressed = true;
	mouse_inside = true;
	long_press_timer.start (KITCHENTIMER_LONG_PRESS_TIMEOUT_MS);
	update ();
	emit pressed ();
    }
}
void ButtonStick::mouseReleaseEvent (QMouseEvent *event)
{
    if (event->button () == Qt::LeftButton) {
	long_press_timer.stop ();
	if (audio_pressed) {
	    audio_pressed = false;
	    update ();
	    if (mouse_inside)
		emit released ();
	}
    }
}
void ButtonStick::mouseMoveEvent (QMouseEvent *event)
{
    mouse_inside = rect ().contains (event->pos ());
    if (audio_pressed)
	update ();
}
void ButtonStick::paintEvent (QPaintEvent*)
{
    QPainter p (this);
    p.drawImage (cached_image.rect (), cached_image, cached_image.rect ());
    if (audio_enabled) {
	if (audio_pressed && mouse_inside)
	    resource_manager->audio_enabled_pressed_svg.render (&p);
	else
	    resource_manager->audio_enabled_svg.render (&p);
    } else {
	if (audio_pressed && mouse_inside)
	    resource_manager->audio_disabled_pressed_svg.render (&p);
	else
	    resource_manager->audio_disabled_svg.render (&p);
    }
}
void ButtonStick::privateLongPressed ()
{
    if (mouse_inside)
	emit longPressed ();
    audio_pressed = false;
    update ();
}
