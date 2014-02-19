#include "audiosequencer.h"
#include "applicationmanager.h"

#include <QAudioOutput>
#include <QFile>

#define SLIDE_CHANNEL_COUNT 16


AudioChannel::AudioChannel (QAudioFormat format)
    : enabled (true), looped (false)
{
    audio_output = new QAudioOutput (format);
    source_file = new QFile;
    connect (audio_output, SIGNAL (stateChanged (QAudio::State)), this, SLOT (handleAudioOutputStateChanged (QAudio::State)));
}
AudioChannel::~AudioChannel ()
{
    audio_output->setVolume (0.0);
    audio_output->reset ();
    delete audio_output;
    delete source_file;
}   
void AudioChannel::play (const QString &source_name)
{
    looped = false;
    audio_output->reset ();
    source_file->close ();
    if (enabled) {
	source_file->setFileName (source_name);
	source_file->open (QIODevice::ReadOnly);
	audio_output->start (source_file);
    }
}
void AudioChannel::playLooped (const QString &source_name)
{
    looped = true;
    audio_output->reset ();
    source_file->close ();
#ifdef KITCHENTIMER_DEBUG_BUILD
    network_log->log (QString ().sprintf ("AudioChannel::playLooped (), enabled: %d", int (enabled)));
#endif
    if (enabled) {
	source_file->setFileName (source_name);
	source_file->open (QIODevice::ReadOnly);
	audio_output->start (source_file);
    }
}
void AudioChannel::stop ()
{
    looped = false;
    audio_output->reset ();
    source_file->close ();
}
void AudioChannel::setEnabled (bool new_enabled)
{
    if (enabled == new_enabled)
	return;
    enabled = new_enabled;
    if (enabled) {
	audio_output->setVolume (1.0);
    } else {
	audio_output->setVolume (0.0);
	audio_output->reset ();
	source_file->close ();
    }
}
void AudioChannel::handleAudioOutputStateChanged (QAudio::State new_state)
{
    if ((new_state == QAudio::IdleState) && looped && enabled) {
	source_file->reset ();
	audio_output->start (source_file);
    }
}


AudioWorker::AudioWorker (bool audio_enabled)
    : audio_enabled (audio_enabled), current_slide_channel (0)
{
    QAudioFormat format;

    format.setSampleRate (22050);
    format.setChannelCount (1);
    format.setSampleSize (16);
    format.setCodec ("audio/pcm");
    format.setByteOrder (QAudioFormat::LittleEndian);
    format.setSampleType (QAudioFormat::SignedInt);

    QAudioDeviceInfo info (QAudioDeviceInfo::defaultOutputDevice ());
    if (!info.isFormatSupported (format))
        qCritical ("Raw audio format not supported by backend, cannot play audio.");

    alarm_channel = new AudioChannel (format);
    event_channel = new AudioChannel (format);
    click_channel = new AudioChannel (format);
    for (int i = 0; i < SLIDE_CHANNEL_COUNT; ++i)
	slide_channel_list.append (new AudioChannel (format));
}
AudioWorker::~AudioWorker ()
{
    delete alarm_channel;
    delete event_channel;
    delete click_channel;
    for (int i = 0; i < SLIDE_CHANNEL_COUNT; ++i)
	delete slide_channel_list[i];
}
void AudioWorker::playAlarm ()
{
#ifdef KITCHENTIMER_DEBUG_BUILD
    network_log->log ("AudioWorker::playAlarm ()");
#endif
    alarm_channel->playLooped (":/audio/alarm.pcm");
}
void AudioWorker::playManualAlarm ()
{
    event_channel->play (":/audio/alarm-manual.pcm");
}
void AudioWorker::playTimerStart ()
{
    event_channel->play (":/audio/timer-start.pcm");
}
void AudioWorker::playAnalogTimerPress ()
{
    click_channel->play (":/audio/analog-timer-press.pcm");
}
void AudioWorker::playAnalogTimerRelease ()
{
    click_channel->play (":/audio/analog-timer-release.pcm");
}
void AudioWorker::playAnalogTimerSlide ()
{
    slide_channel_list[current_slide_channel]->play (QString ().sprintf (":/audio/analog-timer-slide/%02d.pcm", qrand ()%14 + 1));
    current_slide_channel = (current_slide_channel + 1)%SLIDE_CHANNEL_COUNT;
}
void AudioWorker::playAudioEnabledSignal ()
{
    event_channel->play (":/audio/audio-enabled.pcm");
}
void AudioWorker::setAudioEnabled (bool new_audio_enabled)
{
    if (audio_enabled == new_audio_enabled)
	return;
    audio_enabled = new_audio_enabled;
    alarm_channel->setEnabled (audio_enabled);
    event_channel->setEnabled (audio_enabled);
    click_channel->setEnabled (audio_enabled);
    for (int i = 0; i < SLIDE_CHANNEL_COUNT; ++i)
	slide_channel_list[i]->setEnabled (audio_enabled);
    if (audio_enabled)
	playAudioEnabledSignal ();
}
void AudioWorker::stopAlarm ()
{
#ifdef KITCHENTIMER_DEBUG_BUILD
    network_log->log ("AudioWorker::stopAlarm ()");
#endif
    alarm_channel->stop ();
}


AudioSequencer::AudioSequencer ()
{
}
AudioSequencer::~AudioSequencer ()
{
}
void AudioSequencer::run ()
{
    AudioWorker *audio_worker = new AudioWorker (true);
    
    audio_worker->setAudioEnabled (app_manager->getAudioEnabled ());

    connect (this, SIGNAL (enqueueAlarm ()), audio_worker, SLOT (playAlarm ()));
    connect (this, SIGNAL (enqueueManualAlarm ()), audio_worker, SLOT (playManualAlarm ()));
    connect (this, SIGNAL (enqueueTimerStart ()), audio_worker, SLOT (playTimerStart ()));

    connect (this, SIGNAL (enqueueAnalogTimerPress ()), audio_worker, SLOT (playAnalogTimerPress ()));
    connect (this, SIGNAL (enqueueAnalogTimerRelease ()), audio_worker, SLOT (playAnalogTimerRelease ()));
    connect (this, SIGNAL (enqueueAnalogTimerSlide ()), audio_worker, SLOT (playAnalogTimerSlide ()));

    connect (app_manager, SIGNAL (valueChangedAudioEnabled (bool)), audio_worker, SLOT (setAudioEnabled (bool)));
    connect (this, SIGNAL (enqueueStopAlarm ()), audio_worker, SLOT (stopAlarm ()));

    exec ();

    delete audio_worker;
}
