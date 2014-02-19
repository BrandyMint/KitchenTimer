// -*- mode: c++ -*-

#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include "timer.h"
#include "referencemodel.h"
#include "alarmsequencer.h"

#include <QIcon>

#define app_manager ((ApplicationManager*) ApplicationManager::getInstance ())


#ifdef KITCHENTIMER_DEBUG_BUILD
#define network_log ((NetworkLog*) NetworkLog::getInstance ())

QT_BEGIN_NAMESPACE
class QTcpSocket;
QT_END_NAMESPACE

class NetworkLog: public QObject
{
    Q_OBJECT

public:
    static NetworkLog *getInstance ();

public:
    NetworkLog ();
    ~NetworkLog ();
    void write (const QByteArray&);
    void write (const QString&);
    void log (const QString&);

private:
    QTcpSocket *socket;
};
#endif


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
