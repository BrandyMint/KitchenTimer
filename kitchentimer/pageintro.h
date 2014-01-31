// -*- mode: c++ -*-

#ifndef PAGEINTRO_H
#define PAGEINTRO_H

#include "widgets/background.h"

QT_BEGIN_NAMESPACE
class QMouseEvent;
class QTimer;
QT_END_NAMESPACE

class PageIntro: public Background
{
    Q_OBJECT

public:
    PageIntro (QWidget*);
    ~PageIntro ();

protected:
    void mouseReleaseEvent (QMouseEvent*);

private slots:
    void skipIntro ();

private:
    QFont font;

signals:
    void switchToPageTimers ();
};

#endif
