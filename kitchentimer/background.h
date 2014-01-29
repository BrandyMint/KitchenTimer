// -*- mode: c++ -*-

#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QPaintEvent;
QT_END_NAMESPACE

class Background: public QWidget
{
    Q_OBJECT

public:
    Background (QWidget*);

protected:
    void paintEvent (QPaintEvent*);
};

#endif
