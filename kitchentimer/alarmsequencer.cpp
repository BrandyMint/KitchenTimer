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


AlarmSequencer::AlarmSequencer ()
    : last_is_alarm (true), audio_enabled (true)
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
    connect (audio_output, SIGNAL (stateChanged (QAudio::State)), this, SLOT (handleAudioOutputStateChanged (QAudio::State)));
    connect (click_audio_output, SIGNAL (stateChanged (QAudio::State)), this, SLOT (handleAudioOutputStateChanged (QAudio::State)));
}
AlarmSequencer::~AlarmSequencer ()
{
    audio_output->stop ();
    click_audio_output->stop ();
    source_file.close ();
    click_source_file.close ();
    delete audio_output;
    delete click_audio_output;
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
}
void AlarmSequencer::runTimerStart ()
{
    audio_output->stop ();
    source_file.close ();
    if (audio_enabled) {
	source_file.setFileName (":/audio/timer-start.pcm");
	source_file.open (QIODevice::ReadOnly);
	audio_output->start (&source_file);
    }
    last_is_alarm = false;
    enqueueVibration ();
}
void AlarmSequencer::soundAnalogTimerPressed ()
{
    click_audio_output->stop ();
    click_source_file.close ();
    if (audio_enabled) {
	click_source_file.setFileName (":/audio/analog-timer-press.pcm");
	click_source_file.open (QIODevice::ReadOnly);
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
	click_audio_output->start (&click_source_file);
    }
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
    }
}
void AlarmSequencer::enqueueClearAlarms ()
{
    // TODO: Make this one directly if it works
    QTimer::singleShot (0, this, SLOT (clearAlarms ()));
}
void AlarmSequencer::setAudioEnabled (bool new_audio_enabled)
{
    if (audio_enabled == new_audio_enabled)
	return;
    audio_enabled = new_audio_enabled;
    if (audio_enabled) {
	audio_output->setVolume (1.0);
	click_audio_output->setVolume (1.0);
    } else {
	audio_output->setVolume (0.0);
	click_audio_output->setVolume (0.0);
	audio_output->stop ();
	click_audio_output->stop ();
	source_file.close ();
	click_source_file.close ();
    }
}
void AlarmSequencer::enqueueVibration ()
{
#ifdef Q_OS_ANDROID
    QPlatformNativeInterface *interface = QApplication::platformNativeInterface ();
    jobject objActivity = (jobject) interface->nativeResourceForIntegration ("QtActivity");
    
    QAndroidJniEnvironment jniEnv;
    jclass classActivity = jniEnv->FindClass ("android/app/Activity");
    jmethodID mSystemService = jniEnv->GetMethodID (classActivity, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    
    jstring strMethod = jniEnv->NewStringUTF ("vibrator");
    jobject objVibrator = jniEnv->CallObjectMethod (objActivity, mSystemService, strMethod);
    
    jclass classVibrator = jniEnv->GetObjectClass (objVibrator);
    jmethodID mVibrate2 = jniEnv->GetMethodID (classVibrator, "vibrate", "([JI)V");
    
    jlong buf[] = {
	96, 158 - 96,
	313 - 158, 368 - 313,
	557 - 368, 607 - 557,
	775 - 607, 824 - 775,
	1017 - 824, 1065 - 1017,
	1234 - 1065, 1273 - 1234,
	1478 - 1273, 1511 - 1478,
	1703 - 1511, 1729 - 1703,
    };
    jlongArray pattern = jniEnv->NewLongArray (sizeof (buf)/sizeof (buf[0]));
    jniEnv->SetLongArrayRegion (pattern, 0, sizeof (buf)/sizeof (buf[0]), buf);
    jniEnv->CallVoidMethod (objVibrator, mVibrate2, pattern, -1);
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
