#include "alarmsequencer.h"
#include "applicationmanager.h"

#include <QFile>
#include <QAudioOutput>
#include <QDebug>
#include <QApplication>
#ifdef Q_OS_ANDROID
#  include <QAndroidJniEnvironment>
#  include <QAndroidJniObject>
#  include <qpa/qplatformnativeinterface.h>
#endif

#define SLIDE_CHANNEL_COUNT 16

AlarmSequencer::AlarmSequencer ()
    : current_slide_channel (0), last_is_alarm (true), audio_enabled (true), last_vibration_looped (false)
{
    QAudioFormat format;

    format.setSampleRate (22050);
    format.setChannelCount (1);
    format.setSampleSize (16);
    format.setCodec ("audio/pcm");
    format.setByteOrder (QAudioFormat::LittleEndian);
    format.setSampleType (QAudioFormat::SignedInt);

    QAudioDeviceInfo info (QAudioDeviceInfo::defaultOutputDevice ());
    if (!info.isFormatSupported (format)) {
        qFatal ("Raw audio format not supported by backend, cannot play audio.");
	qApp->exit (1);
    }
    
    audio_output = new QAudioOutput (format, NULL);
    click_audio_output = new QAudioOutput (format, NULL);
    for (int i = 0; i < SLIDE_CHANNEL_COUNT; ++i) {
	slide_audio_output_list.append (new QAudioOutput (format, NULL));
	slide_source_file_list.append (new QFile (this));
    }
    // connect (audio_output, SIGNAL (stateChanged (QAudio::State)), this, SLOT (handleAudioOutputStateChanged (QAudio::State)));
    // connect (click_audio_output, SIGNAL (stateChanged (QAudio::State)), this, SLOT (handleAudioOutputStateChanged (QAudio::State)));

#ifdef Q_OS_ANDROID
    QPlatformNativeInterface *interface = QApplication::platformNativeInterface ();
    jobject objActivity = (jobject) interface->nativeResourceForIntegration ("QtActivity");
    
    jclass classActivity = jniEnv->FindClass ("android/app/Activity");
    jmethodID mSystemService = jniEnv->GetMethodID (classActivity, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    
    jstring strMethod = jniEnv->NewStringUTF ("vibrator");
    objVibrator = jniEnv->CallObjectMethod (objActivity, mSystemService, strMethod);
    
    jclass classVibrator = jniEnv->GetObjectClass (objVibrator);
    mCancel = jniEnv->GetMethodID (classVibrator, "cancel", "()V");
    mVibrate = jniEnv->GetMethodID (classVibrator, "vibrate", "(J)V");
    mVibratePattern = jniEnv->GetMethodID (classVibrator, "vibrate", "([JI)V");
    
    jniEnv->CallVoidMethod (objVibrator, mCancel);

    cancel_vibration_timer.setSingleShot (true);
    connect (&cancel_vibration_timer, SIGNAL (timeout ()), this, SLOT (cancelVibration ()));
#endif
}
AlarmSequencer::~AlarmSequencer ()
{
    audio_output->stop ();
    click_audio_output->stop ();
    for (int i = 0; i < SLIDE_CHANNEL_COUNT; ++i)
	slide_audio_output_list[i]->stop ();
    source_file.close ();
    click_source_file.close ();
    for (int i = 0; i < SLIDE_CHANNEL_COUNT; ++i)
	slide_source_file_list[i]->close ();
    delete audio_output;
    delete click_audio_output;
    for (int i = 0; i < SLIDE_CHANNEL_COUNT; ++i)
	delete slide_audio_output_list[i];
}
void AlarmSequencer::runSingleAlarm ()
{
    audio_output->stop ();
    source_file.close ();
    if (audio_enabled) {
	source_file.setFileName (":/audio/alarm.pcm");
	source_file.open (QIODevice::ReadOnly);
	audio_output->start (&source_file);
    }
    last_is_alarm = true;
#ifdef Q_OS_ANDROID
    if (KITCHENTIMER_USE_VIBRATION)
	cancelVibration ();
    int buf[] = {
	10, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, // 100
	60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, // 200
	60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, // 300
	60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, // 400
	60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, // 500
	60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, // 600
	60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, // 700
	60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, 60, 40, // 780
    };
    if (KITCHENTIMER_USE_VIBRATION)
	enqueueVibrationPattern (sizeof (buf)/sizeof (buf[0]), buf, -1);
#endif
}
void AlarmSequencer::runManualAlarm ()
{
    audio_output->stop ();
    source_file.close ();
    if (audio_enabled) {
	source_file.setFileName (":/audio/alarm-manual.pcm");
	source_file.open (QIODevice::ReadOnly);
	audio_output->start (&source_file);
    }
    last_is_alarm = false;
#ifdef Q_OS_ANDROID
    if (KITCHENTIMER_USE_VIBRATION)
	cancelVibration ();
    int buf[] = {
	10, 40, 60, 40, 60, 40, 60, 40, // 40
    };
    if (KITCHENTIMER_USE_VIBRATION)
	enqueueVibrationPattern (sizeof (buf)/sizeof (buf[0]), buf, -1);
#endif
}
void AlarmSequencer::runTimerStart ()
{
    audio_output->stop ();
    source_file.close ();
    if (audio_enabled) {
	source_file.setFileName (":/audio/timer-start.pcm");
	source_file.open (QIODevice::ReadOnly);
	audio_output->setBufferSize (source_file.size ());
	audio_output->start (&source_file);
    }
    last_is_alarm = false;
#ifdef Q_OS_ANDROID
    if (KITCHENTIMER_USE_VIBRATION)
	cancelVibration ();
    int buf[] = {
	96, 158 - 96,
	313 - 158, 368 - 313,
	557 - 368, 607 - 557,
	775 - 607, 824 - 775,
	1017 - 824, 1065 - 1017,
	1234 - 1065, 1273 - 1234,
	1478 - 1273, 1511 - 1478,
	1703 - 1511, 1729 - 1703,
    };
    if (KITCHENTIMER_USE_VIBRATION)
	enqueueVibrationPattern (sizeof (buf)/sizeof (buf[0]), buf, -1);
#endif
}
void AlarmSequencer::soundAnalogTimerPressed ()
{
    click_audio_output->stop ();
    click_source_file.close ();
    if (audio_enabled) {
	click_source_file.setFileName (":/audio/analog-timer-press.pcm");
	click_source_file.open (QIODevice::ReadOnly);
	click_audio_output->setBufferSize (click_source_file.size ());
	click_audio_output->start (&click_source_file);
    }
}
void AlarmSequencer::soundAnalogTimerReleased ()
{
    click_audio_output->stop ();
    click_source_file.close ();
    if (audio_enabled) {
	click_source_file.setFileName (":/audio/analog-timer-release.pcm");
	click_source_file.open (QIODevice::ReadOnly);
	click_audio_output->setBufferSize (click_source_file.size ());
	click_audio_output->start (&click_source_file);
    }
}
void AlarmSequencer::soundAnalogTimerSlide ()
{
    if (KITCHENTIMER_USE_ANALOG_TIMER_SLIDE_SOUND) {
	slide_audio_output_list[current_slide_channel]->stop ();
	slide_source_file_list[current_slide_channel]->close ();
	if (audio_enabled) {
	    slide_source_file_list[current_slide_channel]->setFileName (QString ().sprintf (":/audio/analog-timer-slide/%02d.pcm", qrand ()%14 + 1));
	    slide_source_file_list[current_slide_channel]->open (QIODevice::ReadOnly);
	    slide_audio_output_list[current_slide_channel]->setBufferSize (slide_source_file_list[current_slide_channel]->size ());
	    slide_audio_output_list[current_slide_channel]->start (slide_source_file_list[current_slide_channel]);
	}
	current_slide_channel = (current_slide_channel + 1)%SLIDE_CHANNEL_COUNT;
    }
#ifdef Q_OS_ANDROID
    if (!last_vibration_looped) {
    	cancel_vibration_timer.stop ();
    	int buf[] = {
    	    KITCHENTIMER_SLIDE_VIBRATION_SILENCE_DURATION, KITCHENTIMER_SLIDE_VIBRATION_VIBRATION_DURATION,
    	};
	if (KITCHENTIMER_USE_VIBRATION)
	    enqueueVibrationPattern (sizeof (buf)/sizeof (buf[0]), buf, 0);
    	last_vibration_looped = true;
    }
    cancel_vibration_timer.start (KITCHENTIMER_SLIDE_VIBRATION_ON_TIMEOUT);
#endif
}
void AlarmSequencer::signalLongPress ()
{
    qWarning ("TODO: Enqueue long press vibration");
}
void AlarmSequencer::clearAlarms ()
{
    if (last_is_alarm) {
	audio_output->stop ();
	source_file.close ();
	last_is_alarm = false;
    }
#ifdef Q_OS_ANDROID
    // if (KITCHENTIMER_USE_VIBRATION)
    // 	cancelVibration ();
#endif
}
void AlarmSequencer::setAudioEnabled (bool new_audio_enabled)
{
    if (audio_enabled == new_audio_enabled)
	return;
    audio_enabled = new_audio_enabled;
    if (audio_enabled) {
	audio_output->setVolume (1.0);
	click_audio_output->setVolume (1.0);
	for (int i = 0; i < SLIDE_CHANNEL_COUNT; ++i)
	    slide_audio_output_list[i]->setVolume (1.0);
    } else {
	audio_output->setVolume (0.0);
	click_audio_output->setVolume (0.0);
	for (int i = 0; i < SLIDE_CHANNEL_COUNT; ++i)
	    slide_audio_output_list[i]->setVolume (0.0);
	audio_output->stop ();
	click_audio_output->stop ();
	for (int i = 0; i < SLIDE_CHANNEL_COUNT; ++i)
	    slide_audio_output_list[i]->stop ();
	source_file.close ();
	click_source_file.close ();
	for (int i = 0; i < SLIDE_CHANNEL_COUNT; ++i)
	    slide_source_file_list[i]->close ();
    }
}
void AlarmSequencer::enqueueVibration (int length_ms)
{
#ifdef Q_OS_ANDROID
    jniEnv->CallVoidMethod (objVibrator, mVibrate, jlong (length_ms));
#endif
}
void AlarmSequencer::enqueueVibrationPattern (int count, const int *buffer, int repeat)
{
#ifdef Q_OS_ANDROID
    jlong buf[count];
    for (int i = 0; i < count; ++i)
    	buf[i] = buffer[i];
    jlongArray pattern = jniEnv->NewLongArray (count);
    jniEnv->SetLongArrayRegion (pattern, 0, count, buf);
    jniEnv->CallVoidMethod (objVibrator, mVibratePattern, pattern, jint (repeat));
#endif
}
void AlarmSequencer::cancelVibration ()
{
#ifdef Q_OS_ANDROID
    cancel_vibration_timer.stop ();
    if (last_vibration_looped) {
	jniEnv->CallVoidMethod (objVibrator, mCancel);
	last_vibration_looped = false;
    }
#endif
}
void AlarmSequencer::handleAudioOutputStateChanged (QAudio::State state)
{
    switch (state) {
    case QAudio::IdleState: {
	QAudio::Error error_state = audio_output->error ();
	switch (error_state) {
	case QAudio::OpenError:
	    qWarning ("Audio output is idle: An error occurred opening the audio device");
	    break;
	case QAudio::IOError:
	    qWarning ("Audio output is idle: An error occurred during read/write of audio device");
	    break;
	case QAudio::FatalError:
	    qWarning ("Audio output is idle: A non-recoverable error has occurred, the audio device is not usable at this time.");
	    break;
	default:
	    break;
	}
    } break;
    case QAudio::StoppedState: {
	QAudio::Error error_state = audio_output->error ();
	switch (error_state) {
	case QAudio::OpenError:
	    qWarning ("Audio output has stopped: An error occurred opening the audio device");
	    break;
	case QAudio::IOError:
	    qWarning ("Audio output has stopped: An error occurred during read/write of audio device");
	    break;
	case QAudio::FatalError:
	    qWarning ("Audio output has stopped: A non-recoverable error has occurred, the audio device is not usable at this time.");
	    break;
	default:
	    break;
	}
    } break;
    default: {
    } break;
    }
}
