#include "application.h"

#include <QFontDatabase>
#include <QSettings>
#include <QFile>


Application::Application (int &argc, char **argv)
    : QApplication (argc, argv),
      previous_timer_icon (":/images/previous-timer.png"),
      next_timer_icon (":/images/next-timer.png"),
      previous_dish_icon (":/images/previous-dish.png"),
      next_dish_icon (":/images/next-dish.png"),
      current_timer_icon (":/images/current-timer.png"),
      timer_list_icon (":/images/timer-list.png"),
      audio_enabled_icon (":/images/audio-enabled.png"),
      audio_disabled_icon (":/images/audio-disabled.png"),
      vibrosignal_enabled_icon (":/images/vibrosignal-enabled.png"),
      vibrosignal_disabled_icon (":/images/vibrosignal-disabled.png"),
      bookmarks_icon (":/images/bookmarks.png"),
      reference_close_icon (":/images/reference-close.png"),
      reference_return_icon (":/images/reference-return.png"),
      reference_search_icon (":/images/reference-search.png"),
      background_image (":/images/background-v1.png"),
      current_timer_index (-1)
{
    QFontDatabase::addApplicationFont (":/fonts/Cartonsix NC.ttf");

    base_font = QFont ("CartonsixNC", 17);
    big_font = QFont ("CartonsixNC", 26);
    intro_font = QFont ("CartonsixNC", 54);

    setFont (base_font);

    QSettings settings (KITCHENTIMER_SETTINGS_COMPANY_NAME, KITCHENTIMER_SETTINGS_PRODUCT_NAME);
    settings.beginGroup ("General");
    audio_enabled = settings.value ("audio_enabled", true).toBool ();
    vibrosignal_enabled = settings.value ("vibrosignal_enabled", true).toBool ();
    current_timer_index = settings.value ("current_timer_index", -1).toInt ();
    settings.endGroup ();
    int size = settings.beginReadArray ("Timers");
    for (int i = 0; i < size; ++i) {
	settings.setArrayIndex (i);
	Timer *new_timer = new Timer (QTime::fromString (settings.value ("period").toString (), "hh:mm:ss"),
				      QTime::fromString (settings.value ("time_left").toString (), "hh:mm:ss"),
				      settings.value ("title").toString ());
	timers.append (new_timer);
    }
    settings.endArray();
    if (!timers.count ()) {
	Timer *new_timer = new Timer (QTime (0, 5, 0), QTime (0, 5, 0), "Default timer");
	timers.append (new_timer);
	current_timer_index = 0;
    }

    QFile file (":/reference/reference.txt");
    file.open (QIODevice::ReadOnly);
    reference_model.loadModelData (QString (file.readAll ()).split (QString ("\n")));
    file.close ();
}
Application::~Application ()
{
    QSettings settings (KITCHENTIMER_SETTINGS_COMPANY_NAME, KITCHENTIMER_SETTINGS_PRODUCT_NAME);
    settings.beginGroup ("General");
    settings.setValue ("audio_enabled", audio_enabled);
    settings.setValue ("vibrosignal_enabled", vibrosignal_enabled);
    settings.setValue ("current_timer_index", current_timer_index);
    settings.endGroup ();
    
    settings.beginWriteArray ("Timers");
    int i = 0;
    for (QList<Timer*>::iterator it = timers.begin (); it != timers.end (); ++it, ++i) {
	Timer *timer = *it;
	settings.setArrayIndex (i);
	settings.setValue ("title", timer->getTitle ());
	settings.setValue ("period", timer->getPeriod ());
	settings.setValue ("time_left", timer->getTimeLeft ());
    }
    settings.endArray ();
}
QFont &Application::getBaseFont ()
{
    return base_font;
}
QFont &Application::getBigFont ()
{
    return big_font;
}
QFont &Application::getIntroFont ()
{
    return intro_font;
}
bool Application::getAudioEnabled ()
{
    return audio_enabled;
}
bool Application::getVibrosignalEnabled ()
{
    return vibrosignal_enabled;
}
ReferenceModel &Application::getReferenceModel ()
{
    return reference_model;
}
void Application::setAudioEnabled (bool new_value)
{
    audio_enabled = new_value;
    emit valueChangedAudioEnabled (audio_enabled);
}
void Application::toggleAudioEnabled ()
{
    audio_enabled = !audio_enabled;
    emit valueChangedAudioEnabled (audio_enabled);
}
void Application::setVibrosignalEnabled (bool new_value)
{
    vibrosignal_enabled = new_value;
    emit valueChangedVibrosignalEnabled (vibrosignal_enabled);
}
void Application::toggleVibrosignalEnabled ()
{
    vibrosignal_enabled = !vibrosignal_enabled;
    emit valueChangedVibrosignalEnabled (vibrosignal_enabled);
}
QList<Timer*> &Application::getTimers ()
{
    return timers;
}
void Application::addTimer (Timer *new_timer)
{
    timers.append (new_timer);
    emit timerAdded (new_timer);
}
int Application::getCurrentTimerIndex ()
{
    return current_timer_index;
}
void Application::setCurrentTimerIndex (int new_current_timer_index)
{
    current_timer_index = new_current_timer_index;
}
void Application::runAlarmOnce ()
{
    alarm_sequencer.runSingleAlarm ();
}
void Application::clearAlarms ()
{
    alarm_sequencer.enqueueClearAlarms ();
}
