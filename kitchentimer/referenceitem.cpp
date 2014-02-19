#include "referenceitem.h"

#include <QStringList>


ReferenceItem::ReferenceItem (const QString &title, const QString &full_title, const QString &description, const QTime &default_time,
			      ReferenceItem *parent)
    : title (title), full_title (full_title), description (description), default_time (default_time),
      plain_index (-1), parent_item (parent)
{
    title_with_child_count = title;
}
ReferenceItem::~ReferenceItem()
{
    qDeleteAll (child_items);
}
void ReferenceItem::appendChild (ReferenceItem *item)
{
    child_items.append (item);
}
ReferenceItem *ReferenceItem::child (int row)
{
    return child_items.value (row);
}
int ReferenceItem::childCount () const
{
    return child_items.count ();
}
const QString &ReferenceItem::getTitle () const
{
    return title;
}
const QString &ReferenceItem::getTitleWithChildCount () const
{
    return title_with_child_count;
}
const QString &ReferenceItem::getFullTitle () const
{
    return full_title;
}
const QString &ReferenceItem::getDescription () const
{
    return description;
}
const QTime &ReferenceItem::getDefaultTime () const
{
    return default_time;
}
int ReferenceItem::getPlainIndex () const
{
    return plain_index;
}
ReferenceItem *ReferenceItem::parent ()
{
    return parent_item;
}
int ReferenceItem::row () const
{
    if (parent_item)
        return parent_item->child_items.indexOf (const_cast<ReferenceItem*> (this));

    return 0;
}
int ReferenceItem::postProcessLeafsRecursively (QList<ReferenceItem*> &plain_list, int &current_plain_index)
{
    if (!child_items.count ()) {
	title_with_child_count = title;
	plain_list.append (this);
	plain_index = current_plain_index;
	++current_plain_index;
	return 1;
    } else {
	int count = 0;
	for (QList<ReferenceItem*>::iterator it = child_items.begin (); it != child_items.end (); ++it) {
	    count += (*it)->postProcessLeafsRecursively (plain_list, current_plain_index);
	}
	title_with_child_count = title + " (" + QString::number (count) + ")";
	return count;
    }
}
