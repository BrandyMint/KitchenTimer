// -*- mode: c++ -*-

#ifndef REFERENCEMODEL_H
#define REFERENCEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class ReferenceItem;

class ReferenceModel: public QAbstractItemModel
{
    Q_OBJECT

public:
    ReferenceModel ();
    ~ReferenceModel ();
    QVariant data (const QModelIndex&, int) const;
    Qt::ItemFlags flags (const QModelIndex&) const;
    QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
    QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
    QModelIndex parent (const QModelIndex&) const;
    int rowCount (const QModelIndex& = QModelIndex ()) const;
    int columnCount (const QModelIndex& = QModelIndex ()) const;
    QList<ReferenceItem*> &getPlainList ();
    int getCurrentIndex ();
    void setCurrentIndex (int);
    void loadModelData (const QStringList&);

private:
    ReferenceItem *root_item;
    QList<ReferenceItem*> plain_list;
    int current_index;
};

#endif
