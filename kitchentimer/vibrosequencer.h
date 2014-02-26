// -*- mode: c++ -*-

#ifndef VIBROSEQUENCER_H
#define VIBROSEQUENCER_H

#include <QThread>
#include <QTimer>
#ifdef Q_OS_ANDROID
#  include <QAndroidJniEnvironment>
#  include <QAndroidJniObject>
#  include <qpa/qplatformnativeinterface.h>
#endif

class VibroWorker: public QThread
{
    Q_OBJECT

#ifdef Q_OS_ANDROID
public:
    VibroWorker ();
    ~VibroWorker ();

public slots:
    void playAlarm ();
    void playTimerStart ();

    void playAnalogTimerPress ();
    void playAnalogTimerRelease ();
    void playAnalogTimerSlide ();
    
    void setAudioEnabled (bool);
    void stopAlarm ();

private:
    void enqueueVibration (int);
    void enqueueVibrationPattern (int, const int*, int);

private slots:
    void cancelLoopedVibration ();

private:
    bool last_is_alarm;
    bool last_vibration_looped;
    QAndroidJniEnvironment jni_env;
    jmethodID mCancel;
    jmethodID mVibrate;
    jmethodID mVibratePattern;
    jobject objVibrator;
    QTimer cancel_vibration_timer;
#elif defined (Q_OS_MAC)
public:
    VibroWorker ();
    ~VibroWorker ();
    
public slots:
    void playAlarm ();
    void playTimerStart ();

    void setAudioEnabled (bool);
    void stopAlarm ();
    
private slots:
    void callSingleVibration ();
    
private:
    QTimer alarm_repeater;
#endif
};


class VibroSequencer: public QThread
{
    Q_OBJECT

public:
    VibroSequencer ();
    ~VibroSequencer ();

protected:
    void run ();

signals:
    void enqueueAlarm ();
    void enqueueTimerStart ();

    void enqueueAnalogTimerPress ();
    void enqueueAnalogTimerRelease ();
    void enqueueAnalogTimerSlide ();

    void enqueueStopAlarm ();
};

#endif
