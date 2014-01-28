#include "pageintro.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>


PageIntro::PageIntro (QWidget *parent)
    : QWidget (parent)
{
    QTimer::singleShot (KITCHENTIMER_INTRO_TIMEOUT_MS, this, SLOT (skipIntro ()));

    QVBoxLayout *layout = new QVBoxLayout (this);
    layout->addWidget (new QLabel ("INTRO PAGE", this));
}
PageIntro::~PageIntro ()
{
}
void PageIntro::mouseReleaseEvent (QMouseEvent*)
{
    skipIntro ();
}
void PageIntro::skipIntro ()
{
    emit switchToPageTimers ();
}
