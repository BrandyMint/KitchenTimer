// -*- mode: c++ -*-

#ifndef ALARMSEQUENCER_H
#define ALARMSEQUENCER_H

#include <QFile>
#include <QAudio>

QT_BEGIN_NAMESPACE
class QAudioOutput;
QT_END_NAMESPACE

class AlarmEvent: public QObject
{
    Q_OBJECT

public:
    AlarmEvent ();
    ~AlarmEvent ();
    void start ();
    void stop ();

private slots:
    void handleAudioOutputStateChanged (QAudio::State);

private:
    QFile source_file;
    QAudioOutput *audio_output;

signals:
    void playBackDone ();
};

class AlarmSequencer: public QObject
{
    Q_OBJECT

public:
    AlarmSequencer ();
    ~AlarmSequencer ();
    void runSingleAlarm ();

public slots:
    void clearAlarms ();
    void enqueueClearAlarms ();

private:
    AlarmEvent *event;
};

#endif
