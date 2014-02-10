#ifndef DIGITALTIMER_H
#define DIGITALTIMER_H

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
    void setTime (const QTime&);
    void updateTime ();

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

private slots:
    void checkUpdate ();

private:
    enum ButtonPressed {
	NonePressed,
	AddMinutePressed,
	SubtractMinutePressed,
	ScrollMinutePressed,
    };

    QFont font;
    QFont font2;
    QTime value;
    bool edit_mode;
    QElapsedTimer edit_mode_elapsed_timer;
    int unblock_timeout;
    enum ButtonPressed button_pressed;
    QRect button_pressed_rect;
    QPoint button_pressed_point;
    QPoint previous_point;
    int vertical_scroll_accum;
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
    QTimer repaint_timer;
    QPoint animation_center;

signals:
    void enterEditModeRequested ();
    void timeChanged (const QTime&);
    void lmb_pressed ();
    void lmb_released ();
    void userIsAlive ();
};

#endif
