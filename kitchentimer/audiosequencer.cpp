#include "audiosequencer.h"
#include "applicationmanager.h"
#ifdef Q_OS_MAC
#  include "ios_common.h"
#else
#  include <QAudioOutput>
#  include <QFile>
#endif


#ifndef Q_OS_MAC
AudioChannel::AudioChannel (QAudioFormat format)
    : enabled (true), looped (false), looped_list (false)
{
    audio_output = new QAudioOutput (format);
    source_file = new QFile;
    stop_timer.setSingleShot (true);
    connect (audio_output, SIGNAL (stateChanged (QAudio::State)), this, SLOT (handleAudioOutputStateChanged (QAudio::State)));
    connect (&stop_timer, SIGNAL (timeout ()), this, SLOT (cancelLoopedList ()));
}
AudioChannel::~AudioChannel ()
{
    audio_output->setVolume (0.0);
    audio_output->reset ();
    stop_timer.stop ();
    if (source_file->isOpen ())
	source_file->close ();
    delete audio_output;
    delete source_file;
}   
void AudioChannel::play (const QString &source_name)
{
    looped = false;
    looped_list = false;
    audio_output->reset ();
    stop_timer.stop ();
    if (enabled) {
	if (source_file->isOpen ())
	    source_file->close ();
	source_file->setFileName (source_name);
	source_file->open (QIODevice::ReadOnly);
	audio_output->start (source_file);
    }
}
void AudioChannel::playLooped (const QString &source_name)
{
    looped = true;
    looped_list = false;
    source_name_list.clear ();
    audio_output->reset ();
    stop_timer.stop ();
    if (enabled) {
	if (source_file->isOpen ())
	    source_file->close ();
	source_file->setFileName (source_name);
	source_file->open (QIODevice::ReadOnly);
	audio_output->start (source_file);
    }
}
void AudioChannel::initLoopedList (const QStringList &in_source_name_list, int in_stop_timeout_ms)
{
    looped = false;
    looped_list = false;
    source_name_list = in_source_name_list;
    stop_timeout_ms = in_stop_timeout_ms;
    audio_output->reset ();
    stop_timer.stop ();
}
void AudioChannel::resumeLoopedList ()
{
    if (enabled && source_name_list.size ()) {
	if (looped && looped_list) {
	    stop_timer.start (stop_timeout_ms);
	} else {
	    looped = true;
	    looped_list = true;
	    audio_output->reset ();
	    if (source_file->isOpen ())
		source_file->close ();
	    int index = qrand ()%source_name_list.size ();
	    source_file->setFileName (source_name_list[index]);
	    source_file->open (QIODevice::ReadOnly);
	    audio_output->start (source_file);
	    stop_timer.start (stop_timeout_ms);
	}
    }
}
void AudioChannel::keepAlive ()
{
    if (stop_timer.isActive ())
	stop_timer.start (stop_timeout_ms);
}
void AudioChannel::stop ()
{
    looped = false;
    looped_list = false;
    audio_output->reset ();
    stop_timer.stop ();
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
	stop ();
    }
}
void AudioChannel::handleAudioOutputStateChanged (QAudio::State new_state)
{
    if ((new_state == QAudio::IdleState) && looped && enabled) {
	if (looped_list) {
	    if (source_name_list.size ()) {
		int index = qrand ()%source_name_list.size ();
		if (source_file->isOpen ())
		    source_file->close ();
		source_file->setFileName (source_name_list[index]);
		source_file->open (QIODevice::ReadOnly);
		audio_output->start (source_file);
	    }
	} else {
	    source_file->reset ();
	    audio_output->start (source_file);
	}
    }
}
void AudioChannel::cancelLoopedList ()
{
    looped = false;
    looped_list = false;
}
#endif

#ifdef Q_OS_MAC
AudioWorker::AudioWorker ()
#else
AudioWorker::AudioWorker (QAudioFormat format)
#endif
: audio_enabled (true)
{
#ifdef Q_OS_MAC
    ios_alarm_channel = av_audio_channel_create ();
    ios_event_channel = av_audio_channel_create ();
    ios_click_channel = av_audio_channel_create ();
    const char* source_names[] = {
	"audio/analog-timer-slide-01.wav",
	"audio/analog-timer-slide-02.wav",
	"audio/analog-timer-slide-03.wav",
	"audio/analog-timer-slide-04.wav",
	"audio/analog-timer-slide-05.wav",
	"audio/analog-timer-slide-06.wav",
	"audio/analog-timer-slide-07.wav",
	"audio/analog-timer-slide-08.wav",
	"audio/analog-timer-slide-09.wav",
	"audio/analog-timer-slide-10.wav",
	"audio/analog-timer-slide-11.wav",
	"audio/analog-timer-slide-12.wav",
	"audio/analog-timer-slide-13.wav",
	"audio/analog-timer-slide-14.wav",
    };
    ios_slide_channel = av_audio_channel_create_with_sequencer (14, source_names, 0.25);
#else
    alarm_channel = new AudioChannel (format);
    event_channel = new AudioChannel (format);
    click_channel = new AudioChannel (format);
    slide_channel = new AudioChannel (format);
    QStringList name_list;
    for (int i = 1; i <= 14; ++i)
	name_list.append (QString ().sprintf (":/audio/analog-timer-slide/%02d.pcm", i));
    slide_channel->initLoopedList (name_list, KITCHENTIMER_SLIDE_VIBRATION_ON_TIMEOUT);
#endif
}
AudioWorker::~AudioWorker ()
{
#ifdef Q_OS_MAC
    av_audio_channel_destroy (ios_alarm_channel);
    av_audio_channel_destroy (ios_event_channel);
    av_audio_channel_destroy (ios_click_channel);
    av_audio_channel_destroy (ios_slide_channel);
#else
    delete alarm_channel;
    delete event_channel;
    delete click_channel;
    delete slide_channel;
#endif
}
void AudioWorker::playAlarm ()
{
#ifdef Q_OS_MAC
    av_audio_channel_play (ios_alarm_channel, "audio/alarm.wav", 1);
#else
    alarm_channel->playLooped (":/audio/alarm.pcm");
#endif
}
void AudioWorker::playManualAlarm ()
{
#ifdef Q_OS_MAC
    av_audio_channel_play (ios_event_channel, "audio/alarm-manual.wav", 0);
#else
    event_channel->play (":/audio/alarm-manual.pcm");
#endif
}
void AudioWorker::playTimerStart ()
{
#ifdef Q_OS_MAC
    av_audio_channel_play (ios_event_channel, "audio/timer-start.wav", 0);
#else
    event_channel->play (":/audio/timer-start.pcm");
#endif
}
void AudioWorker::playAnalogTimerPress ()
{
#ifdef Q_OS_MAC
    av_audio_channel_play (ios_click_channel, "audio/analog-timer-press.wav", 0);
#else
    click_channel->play (":/audio/analog-timer-press.pcm");
#endif
}
void AudioWorker::playAnalogTimerRelease ()
{
#ifdef Q_OS_MAC
    av_audio_channel_play (ios_click_channel, "audio/analog-timer-release.wav", 0);
#else
    click_channel->play (":/audio/analog-timer-release.pcm");
#endif
}
void AudioWorker::playAnalogTimerSlide ()
{
#ifdef Q_OS_MAC
    av_audio_channel_play_sequence (ios_slide_channel);
#else
    slide_channel->resumeLoopedList ();
#endif
}
void AudioWorker::playAudioEnabledSignal ()
{
#ifdef Q_OS_MAC
    av_audio_channel_play (ios_event_channel, "audio/audio-enabled.wav", 0);
#else
    event_channel->play (":/audio/audio-enabled.pcm");
#endif
}
void AudioWorker::setAudioEnabled (bool new_audio_enabled)
{
    if (audio_enabled == new_audio_enabled)
	return;
    audio_enabled = new_audio_enabled;
#ifdef Q_OS_MAC
    // Shouldn't forget this
#else
    alarm_channel->setEnabled (audio_enabled);
    event_channel->setEnabled (audio_enabled);
    click_channel->setEnabled (audio_enabled);
    slide_channel->setEnabled (audio_enabled);
#endif
    if (audio_enabled)
	playAudioEnabledSignal ();
}
void AudioWorker::stopAlarm ()
{
#ifdef KITCHENTIMER_DEBUG_BUILD
    network_log->log ("AudioWorker::stopAlarm ()");
#endif
#ifdef Q_OS_MAC
    av_audio_channel_stop (ios_alarm_channel);
#else
    alarm_channel->stop ();
#endif
}


AudioSequencer::AudioSequencer ()
{
}
AudioSequencer::~AudioSequencer ()
{
}
void AudioSequencer::run ()
{
#ifdef Q_OS_MAC
    if (ios_audio_init ()) {
	AudioWorker *audio_worker = new AudioWorker ();
    
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
#else
    QAudioFormat format;

    format.setSampleRate (22050);
    format.setChannelCount (1);
    format.setSampleSize (16);
    format.setCodec ("audio/pcm");
    format.setByteOrder (QAudioFormat::LittleEndian);
    format.setSampleType (QAudioFormat::SignedInt);

    QAudioDeviceInfo info (QAudioDeviceInfo::defaultOutputDevice ());
    if (info.isFormatSupported (format)) {
	AudioWorker *audio_worker = new AudioWorker (format);
    
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
    } else {
        qCritical ("Audio audio format not supported by backend, cannot play audio.");
    }
#endif
}
