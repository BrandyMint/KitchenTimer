// -*- mode: c++ -*-

#ifndef ANALOGTIMER_H
#define ANALOGTIMER_H

#include "animator.h"
#include "editmode.h"

#include <QTime>
#include <QElapsedTimer>
#include <QTimer>
#include <QRect>
#include <QPointF>

class AnalogTimer: public QWidget
{
    Q_OBJECT

public:
    AnalogTimer (QWidget* = NULL);
    ~AnalogTimer ();
    void enterEditMode (int);
    void enterEditModePressed (int, int);
    void leaveEditMode ();
    bool isSliderDown ();
    
protected:
    void resizeEvent (QResizeEvent*);
    void mousePressEvent (QMouseEvent*);
    void mouseReleaseEvent (QMouseEvent*);
    void mouseMoveEvent (QMouseEvent*);
    void paintEvent (QPaintEvent*);

private:
    double getRotationByPos (const QPoint&, const QSize&);

private slots:
    void unhaltEditByTimeout ();

private:
    QFont font;
    QFont small_font;
    QElapsedTimer lifetime_elapsed_timer;
    Animator animator;
    EditMode edit_mode;
    bool down;
    QTime pressed_time;
    double pressed_local_rotation;
    double previous_delta_rotation;
    QTime unhalt_by_timeout_time;
    double unhalt_by_timeout_local_rotation;
    QTimer leave_edit_mode_timer;
    QRect estimated_circle_rect;
    QRect estimated_circle_handle_rect;
    QPointF estimated_circle_center;
    double estimated_circle_radius;
    double estimated_font_pixel_size;

signals:
    void clearAlarms ();
    void enterEditModeRequested ();
    void leaveEditModeRequested ();
    void pressed ();
    void released ();
    void lmb_pressed ();
    void lmb_released ();
    void userIsAlive ();
};
#endif
