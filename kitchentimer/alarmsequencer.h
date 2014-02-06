// -*- mode: c++ -*-

#ifndef ALARMSEQUENCER_H
#define ALARMSEQUENCER_H

#include <QFile>
#include <QAudio>

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
    void runTimerStart ();
    void soundAnalogTimerPressed ();
    void soundAnalogTimerReleased ();
    void signalLongPress ();

public slots:
    void clearAlarms ();
    void enqueueClearAlarms ();
    void setAudioEnabled (bool);

private:
    void enqueueVibration ();

private slots:
    void handleAudioOutputStateChanged (QAudio::State);

private:
    QFile source_file;
    QFile click_source_file;
    QAudioOutput *audio_output;
    QAudioOutput *click_audio_output;
    bool last_is_alarm;
    bool audio_enabled;
};

#endif
