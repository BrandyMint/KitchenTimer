// -*- mode: c++ -*-

#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include "timer.h"
#include "referencemodel.h"
#include "alarmsequencer.h"

#include <QIcon>

#define app_manager ((ApplicationManager*) ApplicationManager::getInstance ())


class ApplicationManager: public QObject
{
    Q_OBJECT

public:
    static ApplicationManager *getInstance ();

public:
    ApplicationManager ();
    ~ApplicationManager ();
    bool getAudioEnabled ();
				 
public slots:
    void setAudioEnabled (bool);
    void toggleAudioEnabled ();
    void setVibrosignalEnabled (bool);
    void toggleVibrosignalEnabled ();
    Timer *getCurrentTimer ();
    void runAlarmOnce ();
    void runTimerStart ();
    void clearAlarms ();

public:
    AlarmSequencer *alarm_sequencer;
    bool edition_happened;
    ReferenceModel reference_model;

private:
    Timer *current_timer;
    bool audio_enabled;
    bool vibrosignal_enabled;

signals:
    void valueChangedAudioEnabled (bool);
    void valueChangedVibrosignalEnabled (bool);
};

#endif
