#include "pageintro.h"

#include <QTimer>


PageIntro::PageIntro (QWidget *parent)
    : IntroBackground (parent)
{
    QTimer::singleShot (KITCHENTIMER_INTRO_TIMEOUT_MS, this, SIGNAL (switchToPageTimers ()));
    connect (this, SIGNAL (pressed ()), this, SIGNAL (switchToPageTimers ()));
}
PageIntro::~PageIntro ()
{
}
