#ifndef ANALOGTIMER_H
#define ANALOGTIMER_H

#include <QDial>
#include <QTime>

class AnalogTimer: public QWidget
{
    Q_OBJECT

public:
    AnalogTimer (QWidget*);
    ~AnalogTimer ();
    void paintEvent (QPaintEvent*);
    void setEditMode (bool);
    QTime getTime ();
    void setTime (const QTime&);
    bool isSliderDown ();
    
protected:
    void mouseMoveEvent (QMouseEvent*);
    void mousePressEvent (QMouseEvent*);
    void mouseReleaseEvent (QMouseEvent*);

private:
    bool edit_mode;
    QTime time_value;
    bool pressed;
    QTime pressed_time;
    double pressed_local_rotation;
    double previous_delta_rotation;
    bool halted;

private:
    void getValuableGeometry (QPoint&, QSize&);
    double getRotationByPos (const QPoint&, const QSize&);

signals:
    void timeChanged (const QTime&);
    void clearAlarms ();
    void enterEditModeRequested ();
    void leaveEditModeRequested ();
};
#endif
