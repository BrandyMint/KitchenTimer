// -*- mode: c++ -*-

#ifndef PAGEDISHSELECT_H
#define PAGEDISHSELECT_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QStackedWidget;
class QTreeView;
class QLabel;
class QPushButton;
QT_END_NAMESPACE


class PageDishSelect: public QWidget
{
    Q_OBJECT

public:
    PageDishSelect (QWidget*);
    ~PageDishSelect ();
    void updateContentSubpageTreeView ();
    void updateContentSubpageDetails ();
    void updateContent ();

private slots:
    void toggleTreeItemExpandCollapse (const QModelIndex&);
    void requestSwitchToPageDishDetails (const QModelIndex&);
    void adjustHyperTimer (const QString&);
    void adjustDefaultTimer ();
    void switchToTreeViewSubpage ();

private:
    QPushButton *switch_to_tree_view_subpage_button;
    QStackedWidget *stacked_widget;
    QWidget *tree_view_subpage;
    QWidget *details_subpage;
    QTreeView *reference_tree_view;
    QLabel *full_title_label;
    QLabel *description_label;
    QPushButton *previous_dish_button;
    QPushButton *next_dish_button;

signals:
    void leavePage ();
    void switchToPageDishDetails (int);
    void previousDish ();
    void nextDish ();
    void setCurrentDish (int);
    void adjustTimer (const QTime&, const QString&);
};

#endif
