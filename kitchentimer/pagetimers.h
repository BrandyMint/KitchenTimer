// -*- mode: c++ -*-

#ifndef PAGETIMERS_H
#define PAGETIMERS_H

#include "widgets/background.h"

#include <QWidget>
#include <QLabel>
#include <QTime>
#include <QTimer>

QT_BEGIN_NAMESPACE
class QPushButton;
class QVBoxLayout;
class QScrollArea;
class QStackedWidget;
QT_END_NAMESPACE

class Timer;
class AnalogTimer;
class ClickableLabel;
class DigitalTimer;

class TimerLabel: public QLabel
{
    Q_OBJECT

public:
    TimerLabel (QWidget*, Timer*);

public slots:
    void updateContent ();
    void timeout ();

private:
    Timer *timer;
};

class PageTimers: public Background
{
    Q_OBJECT

public:
    PageTimers (QWidget*);
    ~PageTimers ();

public slots:
    void updateContentSubpageCurrentTimer ();
    void updateContentSubpageTimerList ();
    void updateContent ();

private slots:
    void setAudioEnabled (bool);
    void setVibrosignalEnabled (bool);
    void setSubpageCurrentTimer ();
    void setSubpageTimerList ();
    void dialValueChanged (int);
    void analogTimeChanged (const QTime&);
    void currentTimerAdjusted ();
    void timerStartStop ();
    void timerReset ();
    void timerEdit ();
    void timerSelect ();
    void timerRemove ();
    void timerMove ();
    void timerBookmark ();
    void clearCurrentAlarms ();
    void switchEditMode (bool);
    void restoreLeaveEditMode ();
    void enterEditMode ();
    void leaveEditMode ();

private:
    QPushButton *toggle_audio_button;
    QPushButton *toggle_vibrosignal_button;
    QPushButton *previous_timer_button;
    QPushButton *next_timer_button;
    QPushButton *goto_timer_button;
    QPushButton *goto_timer_list_button;
    AnalogTimer *analog_timer;
    QStackedWidget *stacked_widget;
    QWidget *current_timer_subpage;
    ClickableLabel *current_timer_title_label;
    DigitalTimer *digital_timer;
    QScrollArea *scroll_area;
    QWidget *scroll_widget;
    QVBoxLayout *scroll_layout;
    QTime dial_value;
    QTimer update_timer;
    QTime saved_period_value;
    QTime saved_time_left_value;
    bool saved_running_state;
    bool edit_mode;

signals:
    void switchToPageTimer ();
    void switchToPageDishSelect ();
    void stopCurrentTimer ();
    void setStartCurrentTimer (const QTime&);
    void previousTimer ();
    void nextTimer ();
    void removeTimer (int);
    void editCurrentTimer ();
};

#endif
