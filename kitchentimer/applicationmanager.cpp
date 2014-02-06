#include "applicationmanager.h"

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
    : QObject (),
      edition_happened (false)
{
    if (instance) {
	qFatal ("Only one instance of ApplicationManager at a time allowed, exiting...");
	qApp->exit (1);
    }

    instance = this;

    QSettings settings (KITCHENTIMER_SETTINGS_COMPANY_NAME, KITCHENTIMER_SETTINGS_PRODUCT_NAME);
    settings.beginGroup ("General");
    audio_enabled = settings.value ("audio_enabled", true).toBool ();
    vibrosignal_enabled = settings.value ("vibrosignal_enabled", true).toBool ();
    current_timer = new Timer (settings.value ("current_timer_time_left", QTime (0, 0, 0)).toTime (), "Default timer");
    settings.endGroup ();

    reference_model.loadModelFile (":/reference/reference.txt");

    alarm_sequencer.setAudioEnabled (audio_enabled);
    connect (this, SIGNAL (valueChangedAudioEnabled (bool)), &alarm_sequencer, SLOT (setAudioEnabled (bool)));

}
ApplicationManager::~ApplicationManager ()
{
    instance = NULL;
    QSettings settings (KITCHENTIMER_SETTINGS_COMPANY_NAME, KITCHENTIMER_SETTINGS_PRODUCT_NAME);
    settings.beginGroup ("General");
    settings.setValue ("audio_enabled", audio_enabled);
    settings.setValue ("vibrosignal_enabled", vibrosignal_enabled);
    settings.setValue ("current_timer_time_left", current_timer->getTimeLeft ());
    settings.endGroup ();
    
    delete current_timer;
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
    alarm_sequencer.runSingleAlarm ();
}
void ApplicationManager::runTimerStart ()
{
    alarm_sequencer.runTimerStart ();
}
void ApplicationManager::clearAlarms ()
{
    alarm_sequencer.enqueueClearAlarms ();
}
void ApplicationManager::soundAnalogTimerPressed ()
{
    alarm_sequencer.soundAnalogTimerPressed ();
}
void ApplicationManager::soundAnalogTimerReleased ()
{
    alarm_sequencer.soundAnalogTimerReleased ();
}
void ApplicationManager::signalLongPress ()
{
    alarm_sequencer.signalLongPress ();
}
