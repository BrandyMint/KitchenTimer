#include "pageintro.h"
#include "application.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>


PageIntro::PageIntro (QWidget *parent)
    : Background (parent), font (qApp->font ())
{
    font.setPointSize (54);
    QTimer::singleShot (KITCHENTIMER_INTRO_TIMEOUT_MS, this, SLOT (skipIntro ()));

    QVBoxLayout *layout = new QVBoxLayout (this);
    QLabel *intro_label = new QLabel ("SMART<br />KITCHEN<br />TIMER", this);
    intro_label->setAlignment (Qt::AlignHCenter | Qt::AlignVCenter);
    intro_label->setFont (font);
    intro_label->setStyleSheet ("QLabel { color : #fcd2a8; }");
    layout->addWidget (intro_label);
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
