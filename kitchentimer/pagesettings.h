// -*- mode: c++ -*-

#ifndef PAGESETTINGS_H
#define PAGESETTINGS_H

#include <QScrollArea>

QT_BEGIN_NAMESPACE
class QSlider;
class QSpinBox;
QT_END_NAMESPACE


class PageSettings: public QScrollArea
{
    Q_OBJECT

public:
    PageSettings (QWidget*);
    ~PageSettings ();

private slots:
    void update_KITCHENTIMER_EDIT_TRANSITION_TIMEOUT_MS (int);
    void update_KITCHENTIMER_EDIT_HOLD_TIMEOUT_MS (int);
    void update_KITCHENTIMER_INITIAL_EDIT_TRANSITION_TIMEOUT_MS (int);
    void update_KITCHENTIMER_INITIAL_EDIT_HOLD_TIMEOUT_MS (int);
    void update_KITCHENTIMER_LEAVE_EDIT_MODE_TIMEOUT_MS (int);
    void update_KITCHENTIMER_ANALOG_TIMER_MODE (int);
    void update_KITCHENTIMER_DIGITAL_TIMER_MODE (int);
    void update_KITCHENTIMER_SHOW_DEBUG_OVERLAY (bool);

private:
    QFont font;
#define ADD_INT_EDIT_VARS(var)	\
    QSlider *slider_ ## var;	\
    QSpinBox *spin_box_ ## var;	
ADD_INT_EDIT_VARS (KITCHENTIMER_EDIT_TRANSITION_TIMEOUT_MS)
ADD_INT_EDIT_VARS (KITCHENTIMER_EDIT_HOLD_TIMEOUT_MS)
ADD_INT_EDIT_VARS (KITCHENTIMER_INITIAL_EDIT_TRANSITION_TIMEOUT_MS)
ADD_INT_EDIT_VARS (KITCHENTIMER_INITIAL_EDIT_HOLD_TIMEOUT_MS)
ADD_INT_EDIT_VARS (KITCHENTIMER_LEAVE_EDIT_MODE_TIMEOUT_MS)

signals:
    void leavePage ();
};

#endif
