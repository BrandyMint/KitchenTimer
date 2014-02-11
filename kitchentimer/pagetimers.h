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
class ButtonStick;


class PageTimers: public Background
{
    Q_OBJECT

public:
    PageTimers (QWidget*);
    ~PageTimers ();

public slots:
    void updateContent ();

protected:
    void resizeEvent (QResizeEvent*);

private slots:
    void clearCurrentAlarms ();
    void enterEditMode ();
    void enterEditModeDigitalTimerPressed ();
    void enterEditModeAnalogTimerPressed ();
    void leaveEditMode ();
    void timeout ();
    void restartTirednessTimer ();
    void startTirednessTimer ();

private:
    ButtonStick *button_stick;
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
    void showSettingsPageRequested ();
};

#endif
