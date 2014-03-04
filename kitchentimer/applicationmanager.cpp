#include "applicationmanager.h"
#ifdef Q_OS_MAC
#  include "ios_common.h"
#endif

#include <QSettings>
#include <QApplication>

static ApplicationManager *instance = NULL;


ApplicationManager *ApplicationManager::getInstance ()
{
    if (!instance) {
	qFatal ("No ApplicationManager instance created, exiting...");
	qApp->exit (1);
    }
    return instance;
}

ApplicationManager::ApplicationManager ()
    : QObject (), edition_happened (false)
{
    if (instance) {
	qFatal ("Only one instance of ApplicationManager at a time allowed, exiting...");
	qApp->exit (1);
    }

    instance = this;

    QSettings settings (KITCHENTIMER_SETTINGS_COMPANY_NAME, KITCHENTIMER_SETTINGS_PRODUCT_NAME);
    settings.beginGroup ("General");
    audio_enabled = true; // TODO: settings.value ("audio_enabled", true).toBool ();
    vibrosignal_enabled = settings.value ("vibrosignal_enabled", true).toBool ();
    current_timer = new Timer (settings.value ("current_timer_time_left", QTime (0, 0, 0)).toTime (), "Default timer");
    settings.endGroup ();

    reference_model.loadModelFile (":/reference/reference.txt");

    alarm_sequencer = new AlarmSequencer;

    connect (current_timer, SIGNAL (timeout ()), this, SLOT (runAlarmOnce ()));

#ifdef Q_OS_MAC
    ios_adjust_idle_timeout ();
#endif
}
ApplicationManager::~ApplicationManager ()
{
    QSettings settings (KITCHENTIMER_SETTINGS_COMPANY_NAME, KITCHENTIMER_SETTINGS_PRODUCT_NAME);
    settings.beginGroup ("General");
    settings.setValue ("audio_enabled", audio_enabled);
    settings.setValue ("vibrosignal_enabled", vibrosignal_enabled);
    settings.setValue ("current_timer_time_left", current_timer->getTimeLeft ());
    settings.endGroup ();
    
    delete alarm_sequencer;
    delete current_timer;

    instance = NULL;
}
bool ApplicationManager::getAudioEnabled ()
{
    return audio_enabled;
}
void ApplicationManager::setAudioEnabled (bool new_value)
{
    audio_enabled = new_value;
    emit valueChangedAudioEnabled (audio_enabled);
}
void ApplicationManager::toggleAudioEnabled ()
{
    audio_enabled = !audio_enabled;
    emit valueChangedAudioEnabled (audio_enabled);
}
void ApplicationManager::setVibrosignalEnabled (bool new_value)
{
    vibrosignal_enabled = new_value;
    emit valueChangedVibrosignalEnabled (vibrosignal_enabled);
}
void ApplicationManager::toggleVibrosignalEnabled ()
{
    vibrosignal_enabled = !vibrosignal_enabled;
    emit valueChangedVibrosignalEnabled (vibrosignal_enabled);
}
Timer *ApplicationManager::getCurrentTimer ()
{
    return current_timer;
}
void ApplicationManager::runAlarmOnce ()
{
    alarm_sequencer->runSingleAlarm ();
}
void ApplicationManager::runTimerStart ()
{
    alarm_sequencer->runTimerStart ();
}
void ApplicationManager::clearAlarms ()
{
    alarm_sequencer->stopAlarm ();
}
