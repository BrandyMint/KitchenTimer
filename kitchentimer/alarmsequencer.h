// -*- mode: c++ -*-

#ifndef ALARMSEQUENCER_H
#define ALARMSEQUENCER_H

#include <QObject>

class AudioSequencer;
class VibroSequencer;


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
    void stopAlarm ();

private:
    AudioSequencer *audio_sequencer;
    VibroSequencer *vibro_sequencer;
};

#endif
