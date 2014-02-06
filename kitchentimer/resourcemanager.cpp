#include "resourcemanager.h"

#include <QApplication>

static ResourceManager *instance = NULL;

ResourceManager *ResourceManager::getInstance ()
{
    if (!instance) {
	qFatal ("No ResourceManager instance created, exiting...");
	qApp->exit (1);
    }
    return instance;
}

ResourceManager::ResourceManager ()
    : QObject (),
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
      background_image (":/images/background.png"),
      button_stick_image (":/images/button-stick.png"),
      audio_enabled_svg (QString (":/images/audio-enabled.svg")),
      audio_enabled_pressed_svg (QString (":/images/audio-enabled-pressed.svg")),
      audio_disabled_svg (QString (":/images/audio-disabled.svg")),
      audio_disabled_pressed_svg (QString (":/images/audio-disabled-pressed.svg")),
      logo_filename (":/images/logo.png")
{
    if (instance) {
	qFatal ("Only one instance of ResourceManager at a time allowed, exiting...");
	qApp->exit (1);
    }
    instance = this;
}
ResourceManager::~ResourceManager ()
{
    instance = NULL;
}
