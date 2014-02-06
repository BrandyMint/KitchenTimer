// -*- mode: c++ -*-

#ifndef INTROBACKGROUND_H
#define INTROBACKGROUND_H

#include <QWidget>
#include <QImage>

class IntroBackground: public QWidget
{
    Q_OBJECT

public:
    IntroBackground (QWidget*);

protected:
    void mousePressEvent (QMouseEvent*);
    void mouseReleaseEvent (QMouseEvent*);
    void paintEvent (QPaintEvent*);

private:
    QImage logo_image;

signals:
    void pressed ();
    void released ();
};

#endif
