#ifndef REFERENCEITEM_H
#define REFERENCEITEM_H

#include <QList>
#include <QVariant>
#include <QTime>

class ReferenceItem
{
public:
    ReferenceItem (const QString&, const QString&, const QString&, const QTime&, ReferenceItem* = 0);
    ~ReferenceItem ();
    void appendChild (ReferenceItem *child);
    ReferenceItem *child (int row);
    int childCount () const;
    int fullChildCount () const;
    const QString &getTitle () const;
    const QString &getTitleWithChildCount () const;
    const QString &getFullTitle () const;
    const QString &getDescription () const;
    const QTime &getDefaultTime () const;
    int getPlainIndex () const;
    int row () const;
    ReferenceItem *parent ();
    int postProcessLeafsRecursively (QList<ReferenceItem*>&, int&);

private:
    QList<ReferenceItem*> child_items;
    QString title;
    QString title_with_child_count;
    QString full_title;
    QString description;
    QTime default_time;
    int plain_index;
    ReferenceItem *parent_item;
};

#endif
