#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include "pagetimer.h"

PageTimer::PageTimer (QWidget *parent)
    : QWidget (parent)
{
    QVBoxLayout *layout = new QVBoxLayout (this);

    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	QPushButton *toggle_audio = new QPushButton ("Audio (ON)", this);
	hlayout->addWidget (toggle_audio);
	QPushButton *toggle_vibration = new QPushButton ("Vibration (ON)", this);
	hlayout->addWidget (toggle_vibration);
	hlayout->addStretch (1);
	layout->addLayout (hlayout);
    }
    
    layout->addStretch (1);

    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	hlayout->addStretch (1);
	QPushButton *goto_timer_button = new QPushButton ("Goto timer", this);
	goto_timer_button->setEnabled (false);
	hlayout->addWidget (goto_timer_button);
	QPushButton *goto_timer_list_button = new QPushButton ("Goto timer list", this);
	connect (goto_timer_list_button, SIGNAL (clicked ()), this, SIGNAL (switchToPageTimerList ()));
	hlayout->addWidget (goto_timer_list_button);
	hlayout->addStretch (1);
	layout->addLayout (hlayout);
    }
    
    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	hlayout->addStretch (1);
	QPushButton *previous_timer_button = new QPushButton ("Previous timer", this);
	hlayout->addWidget (previous_timer_button);
	QPushButton *the_timer_button = new QPushButton ("The timer", this);
	hlayout->addWidget (the_timer_button);
	QPushButton *next_timer_button = new QPushButton ("Next timer", this);
	hlayout->addWidget (next_timer_button);
	hlayout->addStretch (1);
	layout->addLayout (hlayout);
    }
    
    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	hlayout->addStretch (1);
	QPushButton *dish_name_button = new QPushButton ("Dish name", this);
	hlayout->addWidget (dish_name_button);
	hlayout->addStretch (1);
	layout->addLayout (hlayout);
    }
    
    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	hlayout->addStretch (1);
	QPushButton *timeout_button = new QPushButton ("00:05:00", this);
	hlayout->addWidget (timeout_button);
	hlayout->addStretch (1);
	layout->addLayout (hlayout);
    }
    
    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	hlayout->addStretch (1);
	QPushButton *add_timer_button = new QPushButton ("Add timer", this);
	connect (add_timer_button, SIGNAL (clicked ()), this, SIGNAL (switchToPageDishSelect ()));
	hlayout->addWidget (add_timer_button);
	hlayout->addStretch (1);
	layout->addLayout (hlayout);
    }
    
    layout->addStretch (1);
}
PageTimer::~PageTimer ()
{
}
