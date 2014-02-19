// -*- mode: c++ -*-

#ifndef BUTTONSTICK_H
#define BUTTONSTICK_H

#include <QWidget>
#include <QTimer>

class ButtonStick: public QWidget
{
    Q_OBJECT

public:
    ButtonStick (QWidget*);
    ~ButtonStick ();
    void adjustGeometry (const QRect&);
    bool getAudioEnabled ();

public slots:
    void setAudioEnabled (bool);

protected:    
    void resizeEvent (QResizeEvent*);
    void mousePressEvent (QMouseEvent*);
    void mouseReleaseEvent (QMouseEvent*);
    void mouseMoveEvent (QMouseEvent*);
    void paintEvent (QPaintEvent*);

private slots:
    void privateLongPressed ();

private:
    bool audio_enabled;
    bool audio_pressed;
    bool mouse_inside;
    QTimer long_press_timer;
    QImage cached_image;

signals:
    void pressed ();
    void released ();
    void longPressed ();
};

#endif
