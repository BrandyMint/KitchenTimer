#include "alarmsequencer.h"
#include "audiosequencer.h"
#include "vibrosequencer.h"


AlarmSequencer::AlarmSequencer ()
{
    audio_sequencer = new AudioSequencer ();
    vibro_sequencer = new VibroSequencer ();
    audio_sequencer->start ();
    vibro_sequencer->start ();
}
AlarmSequencer::~AlarmSequencer ()
{
    audio_sequencer->exit ();
    vibro_sequencer->exit ();
    audio_sequencer->wait ();
    vibro_sequencer->wait ();
    delete audio_sequencer;
    delete vibro_sequencer;
}
void AlarmSequencer::runSingleAlarm ()
{
    emit audio_sequencer->enqueueAlarm ();
    emit vibro_sequencer->enqueueAlarm ();
}
void AlarmSequencer::runManualAlarm ()
{
    emit audio_sequencer->enqueueManualAlarm ();
}
void AlarmSequencer::runTimerStart ()
{
    emit audio_sequencer->enqueueTimerStart ();
    emit vibro_sequencer->enqueueTimerStart ();
}
void AlarmSequencer::soundAnalogTimerPressed ()
{
    emit audio_sequencer->enqueueAnalogTimerPress ();
    emit vibro_sequencer->enqueueAnalogTimerPress ();
}
void AlarmSequencer::soundAnalogTimerReleased ()
{
    emit audio_sequencer->enqueueAnalogTimerRelease ();
    emit vibro_sequencer->enqueueAnalogTimerRelease ();
}
void AlarmSequencer::soundAnalogTimerSlide ()
{
    emit audio_sequencer->enqueueAnalogTimerSlide ();
    emit vibro_sequencer->enqueueAnalogTimerSlide ();
}
void AlarmSequencer::signalLongPress ()
{
}
void AlarmSequencer::stopAlarm ()
{
    emit audio_sequencer->enqueueStopAlarm ();
    emit vibro_sequencer->enqueueStopAlarm ();
}
