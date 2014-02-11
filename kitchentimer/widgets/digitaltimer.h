// -*- mode: c++ -*-

#ifndef DIGITALTIMER_H
#define DIGITALTIMER_H

#include "animator.h"

#include <QLabel>
#include <QTime>
#include <QTimer>
#include <QElapsedTimer>
#include <QPicture>

class DigitalTimer: public QWidget
{
    Q_OBJECT

public:
    DigitalTimer (QWidget* = NULL);
    ~DigitalTimer ();
    void enterEditMode (int);
    void leaveEditMode ();

protected:    
    void resizeEvent (QResizeEvent*);
    void mouseMoveEvent (QMouseEvent*);
    void mousePressEvent (QMouseEvent*);
    void mouseReleaseEvent (QMouseEvent*);
    void paintEvent (QPaintEvent*);

private:
    QSize charBoundingSize (QPainter&);
    QSize arrowBoundingSize (QPainter&);
    QSize separatorBoundingSize (QPainter&);
    bool addMinuteSharp ();
    bool subtractMinuteSharp ();

private:
    enum ButtonPressed {
	NonePressed,
	LeaveAreaPressed,
	AddMinutePressed,
	SubtractMinutePressed,
	ScrollMinutePressed,
    };

    QFont font;
    QFont font2;
    Animator animator;
    bool edit_mode;
    QElapsedTimer edit_mode_elapsed_timer;
    int unblock_timeout;
    enum ButtonPressed button_pressed;
    QRect button_pressed_rect;
    QPoint button_pressed_point;
    QPoint previous_point;
    int vertical_scroll_accum;
    QPoint animation_center;

    QPicture estimation_picture;
    QSize estimated_size;
    int estimated_font_size;
    QSize estimated_char_bounding_size;
    QSize estimated_arrow_bounding_size;
    QSize estimated_separator_bounding_size;
    int estimated_min_scroll_offset;
    int estimated_scroll_step_offset;
    QRect estimated_minute_add_rect;
    QRect estimated_minute_subtract_rect;
    QRect estimated_digits_rect;

signals:
    void enterEditModeRequested ();
    void leaveEditModeRequested ();
    void lmb_pressed ();
    void lmb_released ();
    void userIsAlive ();
};

#endif
