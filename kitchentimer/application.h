// -*- mode: c++ -*-

#ifndef APPLICATION_H
#define APPLICATION_H

#include "timer.h"
#include "referencemodel.h"
#include "alarmsequencer.h"

#include <QApplication>
#include <QFont>
#include <QIcon>

#define app ((Application*) qApp)

class Application: public QApplication
{
    Q_OBJECT

public:
    Application (int&, char**);
    ~Application ();
    bool getAudioEnabled ();
    bool getVibrosignalEnabled ();
    ReferenceModel &getReferenceModel ();
				 
public slots:
    void setAudioEnabled (bool);
    void toggleAudioEnabled ();
    void setVibrosignalEnabled (bool);
    void toggleVibrosignalEnabled ();
    QList<Timer*> &getTimers ();
    void addTimer (Timer*);
    int getCurrentTimerIndex ();
    Timer *getCurrentTimer ();
    void setCurrentTimerIndex (int);
    void runAlarmOnce ();
    void clearAlarms ();

public:
    QIcon previous_timer_icon;
    QIcon next_timer_icon;
    QIcon previous_dish_icon;
    QIcon next_dish_icon;
    QIcon current_timer_icon;
    QIcon timer_list_icon;
    QIcon audio_enabled_icon;
    QIcon audio_disabled_icon;
    QIcon vibrosignal_enabled_icon;
    QIcon vibrosignal_disabled_icon;
    QIcon bookmarks_icon;
    QIcon reference_close_icon;
    QIcon reference_return_icon;
    QIcon reference_search_icon;
    QImage background_image;

private:
    bool audio_enabled;
    bool vibrosignal_enabled;
    QList<Timer*> timers;
    int current_timer_index;
    ReferenceModel reference_model;
    AlarmSequencer alarm_sequencer;

signals:
    void valueChangedAudioEnabled (bool);
    void valueChangedVibrosignalEnabled (bool);
    void timerAdded (Timer*);
};

#endif
