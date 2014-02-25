#include "vibrosequencer.h"
#include "applicationmanager.h"

#include <QApplication>


VibroWorker::VibroWorker ()
    : last_is_alarm (false), last_vibration_looped (false)
#ifdef Q_OS_ANDROID
    , mCancel (NULL)
    , mVibrate (NULL)
    , mVibratePattern (NULL)
    , objVibrator (NULL)
#endif
{
#ifdef Q_OS_ANDROID
    QPlatformNativeInterface *interface = QApplication::platformNativeInterface ();

    jobject objActivity = (jobject) interface->nativeResourceForIntegration ("QtActivity");
    jclass classActivity = jni_env->FindClass ("android/app/Activity");

    if (objActivity && classActivity) {
	jmethodID mSystemService = jni_env->GetMethodID (classActivity, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    
	jstring strMethod = jni_env->NewStringUTF ("vibrator");
	if (mSystemService && strMethod)
	    objVibrator = jni_env->CallObjectMethod (objActivity, mSystemService, strMethod);
    
	if (objVibrator) {
	    jclass classVibrator = jni_env->GetObjectClass (objVibrator);
	    if (classVibrator) {
		mCancel = jni_env->GetMethodID (classVibrator, "cancel", "()V");
		mVibrate = jni_env->GetMethodID (classVibrator, "vibrate", "(J)V");
		mVibratePattern = jni_env->GetMethodID (classVibrator, "vibrate", "([JI)V");
	    }
	}
    
    }

    cancel_vibration_timer.setSingleShot (true);
    connect (&cancel_vibration_timer, SIGNAL (timeout ()), this, SLOT (cancelLoopedVibration ()));
#endif
}
VibroWorker::~VibroWorker ()
{
}
void VibroWorker::playAlarm ()
{
#ifdef Q_OS_ANDROID
    if (KITCHENTIMER_USE_VIBRATION) {
	cancel_vibration_timer.stop ();
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
	    enqueueVibrationPattern (sizeof (buf)/sizeof (buf[0]), buf, 0);
    }
#endif
    last_is_alarm = true;
}
void VibroWorker::playTimerStart ()
{
#ifdef Q_OS_ANDROID
    if (KITCHENTIMER_USE_VIBRATION) {
	cancel_vibration_timer.stop ();
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
    }
#endif
    last_is_alarm = false;
}
void VibroWorker::playAnalogTimerPress ()
{
    if (KITCHENTIMER_USE_VIBRATION) {
	enqueueVibration (20);
    }
}
void VibroWorker::playAnalogTimerRelease ()
{
    if (KITCHENTIMER_USE_VIBRATION) {
	enqueueVibration (25);
    }
}
void VibroWorker::playAnalogTimerSlide ()
{
#ifdef Q_OS_ANDROID
    if (KITCHENTIMER_USE_VIBRATION) {
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
    }
#endif
}
void VibroWorker::enqueueVibration (int length_ms)
{
#ifdef Q_OS_ANDROID
    if (KITCHENTIMER_USE_VIBRATION) {
	if (jni_env && objVibrator && mVibrate)
	    jni_env->CallVoidMethod (objVibrator, mVibrate, jlong (length_ms));
    }
#endif
}
void VibroWorker::enqueueVibrationPattern (int count, const int *buffer, int repeat)
{
#ifdef Q_OS_ANDROID
    if (KITCHENTIMER_USE_VIBRATION) {
	if (jni_env && objVibrator && mVibratePattern) {
	    jlong buf[count];
	    for (int i = 0; i < count; ++i)
		buf[i] = buffer[i];
	    jlongArray pattern = jni_env->NewLongArray (count);
	    jni_env->SetLongArrayRegion (pattern, 0, count, buf);
	    jni_env->CallVoidMethod (objVibrator, mVibratePattern, pattern, jint (repeat));
	}
    }
#endif
}
void VibroWorker::cancelLoopedVibration ()
{
#ifdef Q_OS_ANDROID
    if (KITCHENTIMER_USE_VIBRATION) {
	cancel_vibration_timer.stop ();
	if (last_vibration_looped) {
	    if (jni_env && objVibrator && mCancel)
		jni_env->CallVoidMethod (objVibrator, mCancel);
	    last_vibration_looped = false;
	    last_is_alarm = false;
	}
    }
#endif
}
void VibroWorker::setAudioEnabled (bool new_audio_enabled)
{
#ifdef Q_OS_ANDROID
    if (KITCHENTIMER_USE_VIBRATION) {
	if (!new_audio_enabled) {
	    cancel_vibration_timer.stop ();
	    int buf[] = {
		20, 80, 20,
	    };
	    if (KITCHENTIMER_USE_VIBRATION)
		enqueueVibrationPattern (sizeof (buf)/sizeof (buf[0]), buf, -1);
	    last_is_alarm = false;
	}
    }
#endif
}
void VibroWorker::stopAlarm ()
{
#ifdef Q_OS_ANDROID
    if (KITCHENTIMER_USE_VIBRATION) {
	if (last_is_alarm) {
	    cancel_vibration_timer.stop ();
	    jni_env->CallVoidMethod (objVibrator, mCancel);
	    last_vibration_looped = false;
	    last_is_alarm = false;
	}
    }
#endif
}


VibroSequencer::VibroSequencer ()
{
}
VibroSequencer::~VibroSequencer ()
{
}
void VibroSequencer::run ()
{
    VibroWorker *vibro_worker = new VibroWorker ();
    
    connect (this, SIGNAL (enqueueAlarm ()), vibro_worker, SLOT (playAlarm ()));
    connect (this, SIGNAL (enqueueTimerStart ()), vibro_worker, SLOT (playTimerStart ()));

    connect (this, SIGNAL (enqueueAnalogTimerPress ()), vibro_worker, SLOT (playAnalogTimerPress ()));
    connect (this, SIGNAL (enqueueAnalogTimerRelease ()), vibro_worker, SLOT (playAnalogTimerRelease ()));
    connect (this, SIGNAL (enqueueAnalogTimerSlide ()), vibro_worker, SLOT (playAnalogTimerSlide ()));

    connect (app_manager, SIGNAL (valueChangedAudioEnabled (bool)), vibro_worker, SLOT (setAudioEnabled (bool)));
    connect (this, SIGNAL (enqueueStopAlarm ()), vibro_worker, SLOT (stopAlarm ()));

    exec ();

    delete vibro_worker;
}
