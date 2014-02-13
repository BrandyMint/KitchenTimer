// -*- mode: c++ -*-

#ifndef ALARMSEQUENCER_H
#define ALARMSEQUENCER_H

#include <QFile>
#include <QAudio>
#ifdef Q_OS_ANDROID
#  include <QTimer>
#  include <QAndroidJniEnvironment>
#  include <QAndroidJniObject>
#  include <qpa/qplatformnativeinterface.h>
#endif

QT_BEGIN_NAMESPACE
class QAudioOutput;
QT_END_NAMESPACE


class AlarmSequencer: public QObject
{
    Q_OBJECT

public:
    AlarmSequencer ();
    ~AlarmSequencer ();
    void runSingleAlarm ();
    void runManualAlarm ();
    void runTimerStart ();
    void soundAnalogTimerPressed ();
    void soundAnalogTimerReleased ();
    void soundAnalogTimerSlide ();
    void signalLongPress ();

public slots:
    void clearAlarms ();
    void setAudioEnabled (bool);

private:
    void enqueueVibration (int);
    void enqueueVibrationPattern (int, const int*, int);

private slots:
    void cancelVibration ();
    void handleAudioOutputStateChanged (QAudio::State);

private:
    QFile source_file;
    QFile click_source_file;
    QList<QFile*> slide_source_file_list;
    QAudioOutput *audio_output;
    QAudioOutput *click_audio_output;
    QList<QAudioOutput*> slide_audio_output_list;
    int current_slide_channel;
    bool last_is_alarm;
    bool audio_enabled;
    bool last_vibration_looped;
#ifdef Q_OS_ANDROID
    QAndroidJniEnvironment jniEnv;
    jmethodID mCancel;
    jmethodID mVibrate;
    jmethodID mVibratePattern;
    jobject objVibrator;
    QTimer cancel_vibration_timer;
#endif
};

#endif
