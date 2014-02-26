// -*- mode: c++ -*-

#ifndef DIGITALTIMER_H
#define DIGITALTIMER_H

#include "animator.h"
#include "editmode.h"

#include <QWidget>
#include <QTime>
#include <QTimer>
#include <QElapsedTimer>
#include <QPicture>

class Background;


class DigitalTimer: public QWidget
{
    Q_OBJECT

public:
    DigitalTimer (Background* = NULL);
    ~DigitalTimer ();
    void enterEditMode (int);
    void enterEditModePressed (int, int);
    void leaveEditMode ();

protected:    
    void resizeEvent (QResizeEvent*);
    void mousePressEvent (QMouseEvent*);
    void mouseReleaseEvent (QMouseEvent*);
    void mouseMoveEvent (QMouseEvent*);
    void paintEvent (QPaintEvent*);

private:
    QSize charBoundingSize (QPainter&);
    QSize arrowBoundingSize (QPainter&);
    QSize separatorBoundingSize (QPainter&);
    bool addMinuteSharp ();
    bool subtractMinuteSharp ();

private slots:
    void unhaltEditByTimeout ();

private:
    enum ButtonPressed {
	NonePressed,
	LeaveAreaPressed,
	AddMinutePressed,
	SubtractMinutePressed,
	ScrollMinutePressed,
    };

    Background *background;
    QFont font;
    QFont font2;
    Animator animator;
    EditMode edit_mode;
    int unblock_timeout;
    bool down;
    QElapsedTimer mouse_move_elapsed_timer;
    double mouse_scroll_step_factor;
    enum ButtonPressed button_pressed;
    QRect button_pressed_rect;
    QPoint button_pressed_point;
    QPoint previous_point;
    double vertical_scroll_accum;
    QPoint animation_center;

    QPicture estimation_picture;
    QSize estimated_size;
    int estimated_font_size;
    QSize estimated_char_bounding_size;
    QSize estimated_arrow_bounding_size;
    QSize estimated_separator_bounding_size;
    double estimated_min_scroll_offset;
    double estimated_scroll_step_offset;
    QRect estimated_minute_add_rect;
    QRect estimated_minute_subtract_rect;
    QRect estimated_minute_scroll_rect;

signals:
    void enterEditModeRequested ();
    void leaveEditModeRequested ();
    void lmb_pressed ();
    void lmb_released ();
    void userIsAlive ();
};

#endif
