#include "alarmsequencer.h"
#include "application.h"

#include <QFile>
#include <QAudioOutput>
#include <QDebug>


AlarmEvent::AlarmEvent ()
{
    source_file.setFileName (":/audio/alarm.pcm");
    source_file.open (QIODevice::ReadOnly);

    QAudioFormat format;

    format.setSampleRate (44100);
    format.setChannelCount (2);
    format.setSampleSize (16);
    format.setCodec ("audio/pcm");
    format.setByteOrder (QAudioFormat::LittleEndian);
    format.setSampleType (QAudioFormat::SignedInt);

    QAudioDeviceInfo info (QAudioDeviceInfo::defaultOutputDevice ());
    if (!info.isFormatSupported (format)) {
        qWarning () << "Raw audio format not supported by backend, cannot play audio.";
	app->quit ();
    }
    
    audio_output = new QAudioOutput (format, NULL);
    connect (audio_output, SIGNAL (stateChanged (QAudio::State)), this, SLOT (handleAudioOutputStateChanged (QAudio::State)));
}
AlarmEvent::~AlarmEvent ()
{
    delete audio_output;
}
void AlarmEvent::start ()
{
    audio_output->stop ();
    source_file.close ();
    source_file.setFileName (":/audio/alarm.pcm");
    source_file.open (QIODevice::ReadOnly);
    audio_output->start (&source_file);
}
void AlarmEvent::stop ()
{
    audio_output->stop ();
    source_file.close ();
}
void AlarmEvent::handleAudioOutputStateChanged (QAudio::State state)
{
    switch (state) {
    case QAudio::IdleState: {
	audio_output->stop ();
	emit playBackDone ();
	// source_file.close ();
	// delete audio_output;
    } break;
    case QAudio::StoppedState: {
	if (audio_output->error () != QAudio::NoError) {
	    qWarning () << "Playback error";
	}
    } break;
    default: {
    } break;
    }
}


AlarmSequencer::AlarmSequencer ()
    : event (NULL)
{
    event = new AlarmEvent ();
    // connect (event, SIGNAL (playBackDone ()), this, SLOT (enqueueClearAlarms ()));
}
AlarmSequencer::~AlarmSequencer ()
{
    delete event;
}
void AlarmSequencer::runSingleAlarm ()
{
    // if (event)
    // 	delete event;
    // event = new AlarmEvent ();
    // event->stop ();
    event->start ();
}
void AlarmSequencer::clearAlarms ()
{
    event->stop ();
    // if (event) {
    // 	delete event;
    // 	event = NULL;
    // }
}
void AlarmSequencer::enqueueClearAlarms ()
{
    QTimer::singleShot (0, this, SLOT (clearAlarms ()));
}
