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

class Background;

class AnalogTimer: public QWidget
{
    Q_OBJECT

public:
    AnalogTimer (Background* = NULL);
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
    Background *background;
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

    QSize estimated_circle_size;
    QRect estimated_circle_rect;
    QRectF estimated_accum_circle_rect;
    QRect estimated_circle_handle_rect;
    QPointF estimated_circle_center;
    double estimated_circle_radius;
    double estimated_font_pixel_size;
    QRadialGradient estimated_current_grad;
    QRadialGradient estimated_current2_grad;
    QRadialGradient estimated_current3_grad;
    QRadialGradient estimated_full_grad;
    int estimated_full_hours;
    QPointF estimated_handle_triangle_points[3];

    QImage cached_back_layer;
    QImage cached_analog_timer_handle_layer;
    QImage cached_over_layer;
#ifdef Q_OS_MAC
    QImage blend_layer;
#endif

signals:
    void clearAlarms ();
    void enterEditModeRequested ();
    void leaveEditModeRequested ();
    void pressed ();
    void released ();
    void slide ();
    void zeroTimeReached ();
    void lmb_pressed ();
    void lmb_released ();
    void userIsAlive ();
};
#endif
