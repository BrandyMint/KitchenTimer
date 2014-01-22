#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>

#include "pageintro.h"

PageIntro::PageIntro (QWidget *parent)
    : QWidget (parent)
{
    skip_intro_timer = new QTimer (this);
    connect (skip_intro_timer, SIGNAL (timeout ()), this, SLOT (skipIntro ()));
    skip_intro_timer->start (KITCHENTIMER_INTRO_TIMEOUT_MS);

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
    if (skip_intro_timer) {
	delete skip_intro_timer;
	skip_intro_timer = NULL;
    }
    emit switchToPageTimer ();
}
