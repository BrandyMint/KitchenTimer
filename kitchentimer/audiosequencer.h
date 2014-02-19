// -*- mode: c++ -*-

#ifndef AUDIOSEQUENCER_H
#define AUDIOSEQUENCER_H

#include <QAudio>
#include <QAudioFormat>
#include <QThread>

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
    void stop ();
    void setEnabled (bool);
    
private slots:
    void handleAudioOutputStateChanged (QAudio::State);

private:
    QAudioOutput *audio_output;
    QFile *source_file;
    bool enabled;
    bool looped;
};

class AudioWorker: public QThread
{
    Q_OBJECT

public:
    AudioWorker (bool);
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
    int current_slide_channel;

    AudioChannel *alarm_channel;
    AudioChannel *event_channel;
    AudioChannel *click_channel;
    QList<AudioChannel*> slide_channel_list;
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
