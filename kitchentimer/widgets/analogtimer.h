// -*- mode: c++ -*-

#ifndef ANALOGTIMER_H
#define ANALOGTIMER_H

#include <QDial>
#include <QTime>
#include <QElapsedTimer>
#include <QTimer>
#include <QRect>
#include <QPointF>

class AnalogTimer: public QWidget
{
    Q_OBJECT

public:
    AnalogTimer (QWidget*);
    ~AnalogTimer ();
    void enterEditMode (int);
    void enterEditModePressed (int, int);
    void leaveEditMode ();
    QTime getTime ();
    void setTime (const QTime&);
    bool isSliderDown ();
    
protected:
    void resizeEvent (QResizeEvent*);
    void mouseMoveEvent (QMouseEvent*);
    void mousePressEvent (QMouseEvent*);
    void mouseReleaseEvent (QMouseEvent*);
    void mouseDoubleClickEvent(QMouseEvent*);
    void paintEvent (QPaintEvent*);

private:
    double getRotationByPos (const QPoint&, const QSize&);

private slots:
    void unblockEdit ();
    void unhaltByTimeoutEdit ();

private:
    QFont font;
    QFont small_font;
    QElapsedTimer lifetime_elapsed_timer;
    bool edit_mode;
    bool edit_blocked;
    QTime time_value;
    bool down;
    QTime pressed_time;
    double pressed_local_rotation;
    double previous_delta_rotation;
    bool halted;
    bool halted_by_timeout;
    QTime unhalt_by_timeout_time;
    double unhalt_by_timeout_local_rotation;
    QTimer leave_edit_mode_timer;
    QRect estimated_circle_rect;
    QRect estimated_circle_handle_rect;
    QPointF estimated_circle_center;
    double estimated_circle_radius;
    double estimated_font_pixel_size;

signals:
    void timeChanged (const QTime&);
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
