#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include "pagedishselect.h"

PageDishSelect::PageDishSelect (QWidget *parent)
    : QWidget (parent)
{
    QVBoxLayout *layout = new QVBoxLayout (this);

    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	hlayout->addStretch (1);
	hlayout->addWidget (new QLabel ("What do we cook?", this));
	QPushButton *close_button = new QPushButton ("Close", this);
	connect (close_button, SIGNAL (clicked ()), this, SIGNAL (leavePage ()));
	hlayout->addWidget (close_button, 1);
	layout->addLayout (hlayout);
    }

    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	QLineEdit *search_line_edit = new QLineEdit (this);
	hlayout->addWidget (search_line_edit, 1);
	QPushButton *search_button = new QPushButton ("Search", this);
	hlayout->addWidget (search_button);
	layout->addLayout (hlayout);
    }

    {
	layout->addWidget (new QLabel ("> Eggs\n> Noodles", this), 1);
    }
    
    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	hlayout->addStretch (1);
	QPushButton *details_button = new QPushButton ("Chicken eggs", this);
	connect (details_button, SIGNAL (clicked ()), this, SIGNAL (switchToPageDishDetails ()));
	hlayout->addWidget (details_button);
	hlayout->addStretch (1);
	layout->addLayout (hlayout);
    }
    
    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	hlayout->addStretch (1);
	QPushButton *adjust_button = new QPushButton ("Adjust timer", this);
	hlayout->addWidget (adjust_button);
	hlayout->addStretch (1);
	layout->addLayout (hlayout);
    }
}
PageDishSelect::~PageDishSelect ()
{
}
