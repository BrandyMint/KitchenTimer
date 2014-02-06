#include "pagedishselect.h"
#include "applicationmanager.h"
#include "resourcemanager.h"
#include "referenceitem.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTreeView>
#include <QStackedWidget>
#include <QScrollArea>

#include <stdio.h>


PageDishSelect::PageDishSelect (QWidget *parent)
    : QWidget (parent)
{
    QVBoxLayout *layout = new QVBoxLayout (this);

    {
	QHBoxLayout *hlayout = new QHBoxLayout ();
	switch_to_tree_view_subpage_button = new QPushButton (resource_manager->reference_return_icon, "", this);
	connect (switch_to_tree_view_subpage_button, SIGNAL (clicked ()), this, SLOT (switchToTreeViewSubpage ()));
	hlayout->addWidget (switch_to_tree_view_subpage_button);
	switch_to_tree_view_subpage_button->setEnabled (false);
	hlayout->addStretch (1);
	hlayout->addWidget (new QLabel ("What do we cook?", this));
	hlayout->addStretch (1);
	QPushButton *close_button = new QPushButton (resource_manager->reference_close_icon, "", this);
	connect (close_button, SIGNAL (clicked ()), this, SIGNAL (leavePage ()));
	hlayout->addWidget (close_button);
	layout->addLayout (hlayout);
    }

    stacked_widget = new QStackedWidget (this);
    { // Tree view subpage
	tree_view_subpage = new QWidget (stacked_widget);
	QVBoxLayout *subpage_layout = new QVBoxLayout (tree_view_subpage);
	{
	    QHBoxLayout *hlayout = new QHBoxLayout ();
	    QLineEdit *search_line_edit = new QLineEdit (tree_view_subpage);
	    hlayout->addWidget (search_line_edit, 1);
	    QPushButton *search_button = new QPushButton (resource_manager->reference_search_icon, "", tree_view_subpage);
	    hlayout->addWidget (search_button);
	    subpage_layout->addLayout (hlayout);
	}
	{
	    reference_tree_view = new QTreeView (tree_view_subpage);
	    reference_tree_view->setModel (&app_manager->reference_model);
	    reference_tree_view->setHeaderHidden (true);
	    reference_tree_view->show ();
	    connect (reference_tree_view, SIGNAL (clicked (const QModelIndex&)),
		     this, SLOT (toggleTreeItemExpandCollapse (const QModelIndex&)));
	    connect (reference_tree_view, SIGNAL (clicked (const QModelIndex&)),
		     this, SLOT (requestSwitchToPageDishDetails (const QModelIndex&)));
	    subpage_layout->addWidget (reference_tree_view, 1);
	}
	stacked_widget->addWidget (tree_view_subpage);
    }
    { // Details subpage
	details_subpage = new QWidget (stacked_widget);
	QVBoxLayout *subpage_layout = new QVBoxLayout (details_subpage);

	{
	    QHBoxLayout *hlayout = new QHBoxLayout ();
	    previous_dish_button = new QPushButton (resource_manager->previous_dish_icon, "", details_subpage);
	    connect (previous_dish_button, SIGNAL (clicked ()), this, SIGNAL (previousDish ()));
	    hlayout->addWidget (previous_dish_button);
	    hlayout->addStretch (1);
	    full_title_label = new QLabel ("Full title", details_subpage);
	    hlayout->addWidget (full_title_label, 0);
	    hlayout->addStretch (1);
	    next_dish_button = new QPushButton (resource_manager->next_dish_icon, "", details_subpage);
	    connect (next_dish_button, SIGNAL (clicked ()), this, SIGNAL (nextDish ()));
	    hlayout->addWidget (next_dish_button);
	    subpage_layout->addLayout (hlayout);
	}

	{
	    QScrollArea *description_scroll_area = new QScrollArea (details_subpage);
	    description_scroll_area->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
	    description_scroll_area->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
	    {
		description_label = new QLabel (description_scroll_area);
		description_label->setAlignment (Qt::AlignLeft | Qt::AlignTop);
		description_label->setWordWrap (true);
		connect (description_label, SIGNAL (linkActivated (const QString&)), this, SLOT (adjustHyperTimer (const QString&)));
		description_scroll_area->setWidget (description_label);
	    }
	    description_scroll_area->setWidgetResizable (true);
	    subpage_layout->addWidget (description_scroll_area, 1);
	}
    
	{
	    QHBoxLayout *hlayout = new QHBoxLayout ();
	    hlayout->addStretch (1);
	    QPushButton *adjust_button = new QPushButton ("Adjust timer", details_subpage);
	    connect (adjust_button, SIGNAL (clicked ()), this, SLOT (adjustDefaultTimer ()));
	    hlayout->addWidget (adjust_button);
	    hlayout->addStretch (1);
	    subpage_layout->addLayout (hlayout);
	}

	stacked_widget->addWidget (details_subpage);
    }
    layout->addWidget (stacked_widget, 1);
}
PageDishSelect::~PageDishSelect ()
{
}
void PageDishSelect::updateContentSubpageTreeView ()
{
}
void PageDishSelect::updateContentSubpageDetails ()
{
    ReferenceModel &reference_model = app_manager->reference_model;
    QList<ReferenceItem*> &plain_list = reference_model.getPlainList ();
    int current_index = reference_model.getCurrentIndex ();
    if ((current_index < 0) || (current_index > plain_list.size ())) {
	full_title_label->setText ("");
	description_label->setText ("");
	return;
    }
    QString full_title = reference_model.getPlainList ()[current_index]->getFullTitle ();
    QString description = reference_model.getPlainList ()[current_index]->getDescription ();
    full_title_label->setText (full_title);
    description_label->setText (description);
    int previous_count = current_index;
    int next_count = plain_list.size () - current_index - 1;
    previous_dish_button->setText ("(" + QString::number (previous_count) + ")");
    previous_dish_button->setEnabled (previous_count > 0);
    next_dish_button->setText ("(" + QString::number (next_count) + ")");
    next_dish_button->setEnabled (next_count > 0);
}
void PageDishSelect::updateContent ()
{
    updateContentSubpageTreeView ();
    updateContentSubpageDetails ();
}
void PageDishSelect::toggleTreeItemExpandCollapse (const QModelIndex &index)
{
    if (!index.isValid ())
        return;
    ReferenceItem *item = static_cast<ReferenceItem*> (index.internalPointer ());
    if (!item->childCount ())
	return;
    if (reference_tree_view->isExpanded (index))
	reference_tree_view->collapse (index);
    else
	reference_tree_view->expand (index);
}
void PageDishSelect::switchToTreeViewSubpage ()
{
    updateContentSubpageTreeView ();
    switch_to_tree_view_subpage_button->setEnabled (false);
    stacked_widget->setCurrentWidget (tree_view_subpage);
}
void PageDishSelect::requestSwitchToPageDishDetails (const QModelIndex &index)
{
    if (!index.isValid ())
        return;
    ReferenceItem *item = static_cast<ReferenceItem*> (index.internalPointer ());
    if (!item->childCount ()) {
	emit setCurrentDish (item->getPlainIndex ());
	updateContentSubpageDetails ();
	switch_to_tree_view_subpage_button->setEnabled (true);
	stacked_widget->setCurrentWidget (details_subpage);
    }
}
void PageDishSelect::adjustHyperTimer (const QString &link)
{
    int current_index = app_manager->reference_model.getCurrentIndex ();
    if ((current_index < 0) || (current_index > app_manager->reference_model.getPlainList ().size ())) {
	return;
    }
    QTime time = QTime::fromString (link, "hh:mm:ss");
    QString full_title = app_manager->reference_model.getPlainList ()[current_index]->getFullTitle ();
    emit adjustTimer (time, full_title);
}
void PageDishSelect::adjustDefaultTimer ()
{
    int current_index = app_manager->reference_model.getCurrentIndex ();
    if ((current_index < 0) || (current_index > app_manager->reference_model.getPlainList ().size ())) {
	return;
    }
    QTime default_time = app_manager->reference_model.getPlainList ()[current_index]->getDefaultTime ();
    QString full_title = app_manager->reference_model.getPlainList ()[current_index]->getFullTitle ();
    emit adjustTimer (default_time, full_title);
}
