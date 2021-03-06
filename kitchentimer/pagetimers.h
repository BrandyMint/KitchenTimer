// -*- mode: c++ -*-

#ifndef PAGETIMERS_H
#define PAGETIMERS_H

#include "widgets/background.h"

#include <QWidget>
#include <QLabel>
#include <QTime>
#include <QTimer>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
QT_END_NAMESPACE

class Timer;
class AnalogTimer;
class ClickableLabel;
class DigitalTimer;


class PageTimers: public Background
{
    Q_OBJECT

public:
    PageTimers (QWidget*);
    ~PageTimers ();

public slots:
    void updateContent ();

private slots:
    void clearCurrentAlarms ();
    void enterEditMode ();
    void enterEditModeDigitalTimerPressed ();
    void enterEditModeAnalogTimerPressed ();
    void leaveEditMode ();
    void restartTirednessTimer ();
    void startTirednessTimer ();

private:
    AnalogTimer *analog_timer;
    DigitalTimer *digital_timer;
    QTime dial_value;
    QTimer update_timer;
    bool edit_mode;
    QTimer leave_edit_mode_timer;

signals:
    void switchToPageTimer ();
    void switchToPageDishSelect ();
    void stopCurrentTimer ();
    void setStartCurrentTimer (const QTime&);
    void previousTimer ();
    void nextTimer ();
    void removeTimer (int);
    void editCurrentTimer ();
    void analogTimerPressed ();
    void analogTimerReleased ();
    void analogTimerSlide ();
    void zeroTimeReached ();
    void longPressed ();
    void showSettingsPageRequested ();
};

#endif
