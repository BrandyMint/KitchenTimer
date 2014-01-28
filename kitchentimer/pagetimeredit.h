// -*- mode: c++ -*-

#ifndef PAGETIMEREDIT_H
#define PAGETIMEREDIT_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QSpinBox;
QT_END_NAMESPACE

class PageTimerEdit: public QWidget
{
    Q_OBJECT

public:
    PageTimerEdit (QWidget*);
    ~PageTimerEdit ();
    void updateContent ();

private slots:
    void acceptTimer ();

private:
    QLineEdit *title_line_edit;
    QSpinBox *hours_spin_box;
    QSpinBox *minutes_spin_box;
    QSpinBox *seconds_spin_box;

signals:
    void cancel ();
    void accept (const QString&, const QTime&);
};

#endif
