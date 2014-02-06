// -*- mode: c++ -*-

#ifndef PAGEINTRO_H
#define PAGEINTRO_H

#include "widgets/introbackground.h"


class PageIntro: public IntroBackground
{
    Q_OBJECT

public:
    PageIntro (QWidget*);
    ~PageIntro ();

signals:
    void switchToPageTimers ();
};

#endif
