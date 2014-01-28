#include "referenceitem.h"
#include "referencemodel.h"

#include <QStringList>

ReferenceModel::ReferenceModel ()
    : current_index (-1)
{
    root_item = new ReferenceItem ("Title", "Full title", "Description", QTime (0, 0, 0));
}
ReferenceModel::~ReferenceModel ()
{
    delete root_item;
}
int ReferenceModel::columnCount (const QModelIndex&) const
{
    return 1;
}
QVariant ReferenceModel::data (const QModelIndex &index, int role) const
{
    if (!index.isValid () || (role != Qt::DisplayRole) || (index.column () != 0))
        return QVariant ();
    return static_cast<ReferenceItem*> (index.internalPointer ())->getTitleWithChildCount ();
}
Qt::ItemFlags ReferenceModel::flags (const QModelIndex &index) const
{
    if (!index.isValid ())
        return 0;

    return QAbstractItemModel::flags (index);
}
QVariant ReferenceModel::headerData (int, Qt::Orientation, int) const
{
    return QVariant ();
}
QModelIndex ReferenceModel::index (int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex (row, column, parent))
        return QModelIndex ();

    ReferenceItem *parent_item;

    if (!parent.isValid ())
        parent_item = root_item;
    else
        parent_item = static_cast<ReferenceItem*> (parent.internalPointer ());

    ReferenceItem *child_item = parent_item->child (row);
    if (child_item)
        return createIndex (row, column, child_item);
    else
        return QModelIndex ();
}
QModelIndex ReferenceModel::parent (const QModelIndex &index) const
{
    if (!index.isValid ())
        return QModelIndex ();

    ReferenceItem *childItem = static_cast<ReferenceItem*> (index.internalPointer());
    ReferenceItem *parent_item = childItem->parent ();

    if (parent_item == root_item)
        return QModelIndex ();

    return createIndex (parent_item->row (), 0, parent_item);
}
int ReferenceModel::rowCount (const QModelIndex &parent) const
{
    ReferenceItem *parent_item;
    if (parent.column () > 0)
        return 0;

    if (!parent.isValid ())
        parent_item = root_item;
    else
        parent_item = static_cast<ReferenceItem*> (parent.internalPointer ());

    return parent_item->childCount ();
}
QList<ReferenceItem*> &ReferenceModel::getPlainList ()
{
    return plain_list;
}
int ReferenceModel::getCurrentIndex ()
{
    return current_index;
}
void ReferenceModel::setCurrentIndex (int new_current_index)
{
    current_index = new_current_index;
}
void ReferenceModel::loadModelData (const QStringList &lines)
{
    QList<ReferenceItem*> parents;
    QList<int> indentations;
    parents << root_item;
    indentations << 0;

    int number = 0;

    for (QStringList::const_iterator it = lines.begin (); it != lines.end (); ++it) {
	QString line = *it;
        int position = 0;

        while (position < line.length ()) {
            if (line.mid (position, 1) != " ")
                break;
            position++;
        }

        line = line.mid (position).trimmed ();

        if (!line.isEmpty ()) {
            QStringList column_strings = line.split ("\t", QString::SkipEmptyParts);
            if (position > indentations.last ()) {
		ReferenceItem *parents_last = parents.last ();
                if (parents_last->childCount () > 0) {
                    parents << parents_last->child (parents_last->childCount () - 1);
                    indentations << position;
                }
            } else {
                while (position < indentations.last () && parents.count () > 0) {
                    parents.pop_back ();
                    indentations.pop_back ();
                }
            }
	    while (column_strings.size () < 4)
		column_strings << "";
	    QTime default_time = QTime::fromString (column_strings[3], "hh:mm:ss");
	    if (!default_time.isValid ())
		default_time = QTime (0, 0, 0);
	    ReferenceItem *reference_item = new ReferenceItem (column_strings[0], column_strings[1], column_strings[2], default_time,
							       parents.last ());
            parents.last ()->appendChild (reference_item);
        }
        ++number;
    }
    int current_plain_index = 0;
    root_item->postProcessLeafsRecursively (plain_list, current_plain_index);
}
