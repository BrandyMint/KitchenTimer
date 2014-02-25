#include "audiosequencer.h"
#include "applicationmanager.h"

#include <QAudioOutput>
#include <QFile>


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


AudioWorker::AudioWorker (QAudioFormat format)
    : audio_enabled (true)
{
    alarm_channel = new AudioChannel (format);
    event_channel = new AudioChannel (format);
    click_channel = new AudioChannel (format);
    slide_channel = new AudioChannel (format);
    QStringList name_list;
    for (int i = 1; i <= 14; ++i)
	name_list.append (QString ().sprintf (":/audio/analog-timer-slide/%02d.pcm", i));
    slide_channel->initLoopedList (name_list, KITCHENTIMER_SLIDE_VIBRATION_ON_TIMEOUT);
}
AudioWorker::~AudioWorker ()
{
    delete alarm_channel;
    delete event_channel;
    delete click_channel;
    delete slide_channel;
}
void AudioWorker::playAlarm ()
{
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
    slide_channel->resumeLoopedList ();
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
    slide_channel->setEnabled (audio_enabled);
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
    QAudioFormat format;

    format.setSampleRate (22050);
#ifdef Q_OS_MAC
    format.setChannelCount (2);
#else
    format.setChannelCount (1);
#endif
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
}
