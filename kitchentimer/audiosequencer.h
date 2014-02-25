// -*- mode: c++ -*-

#ifndef AUDIOSEQUENCER_H
#define AUDIOSEQUENCER_H

#include <QAudio>
#include <QAudioFormat>
#include <QThread>
#include <QStringList>
#include <QTimer>

QT_BEGIN_NAMESPACE
class QAudioOutput;
class QFile;
QT_END_NAMESPACE


class AudioChannel: public QObject
{
    Q_OBJECT

public:
    AudioChannel (QAudioFormat);
    ~AudioChannel ();
    void play (const QString&);
    void playLooped (const QString&);
    void initLoopedList (const QStringList&, int);
    void resumeLoopedList ();
    void setEnabled (bool);
    void keepAlive ();
    void stop ();

private slots:
    void handleAudioOutputStateChanged (QAudio::State);
    void cancelLoopedList ();

private:
    QAudioOutput *audio_output;
    QFile *source_file;
    bool enabled;
    bool looped;
    bool looped_list;
    QTimer stop_timer;
    QStringList source_name_list;
    int stop_timeout_ms;
};

class AudioWorker: public QThread
{
    Q_OBJECT

public:
    AudioWorker (QAudioFormat);
    ~AudioWorker ();

public slots:
    void playAlarm ();
    void playManualAlarm ();
    void playTimerStart ();

    void playAnalogTimerPress ();
    void playAnalogTimerRelease ();
    void playAnalogTimerSlide ();
    void playAudioEnabledSignal ();
    
    void setAudioEnabled (bool);
    void stopAlarm ();

private:
    bool audio_enabled;

    AudioChannel *alarm_channel;
    AudioChannel *event_channel;
    AudioChannel *click_channel;
    AudioChannel *slide_channel;
};


class AudioSequencer: public QThread
{
    Q_OBJECT

public:
    AudioSequencer ();
    ~AudioSequencer ();

protected:
    void run ();

signals:
    void enqueueAlarm ();
    void enqueueManualAlarm ();
    void enqueueTimerStart ();

    void enqueueAnalogTimerPress ();
    void enqueueAnalogTimerRelease ();
    void enqueueAnalogTimerSlide ();

    void enqueueStopAlarm ();
};

#endif
