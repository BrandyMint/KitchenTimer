// -*- mode: c++ -*-

#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "animator.h"

#include <QWidget>
#include <QElapsedTimer>
#include <QTimer>

class Background: public QWidget
{
    Q_OBJECT

public:
    Background (QWidget*);
    void setShaded (bool);
    void startShading (int);
    void startUnshading (int);

protected:
    void mousePressEvent (QMouseEvent*);
    void mouseReleaseEvent (QMouseEvent*);
    void paintEvent (QPaintEvent*);

private:
    Animator animator;
    bool shaded;

signals:
    void pressed ();
    void released ();
    void shadingDone ();
    void unshadingDone ();
};

#endif
