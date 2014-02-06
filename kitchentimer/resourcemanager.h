// -*- mode: c++ -*-

#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QIcon>
#include <QImage>
#include <QSvgRenderer>

#define resource_manager ((ResourceManager*) ResourceManager::getInstance ())


class ResourceManager: public QObject
{
    Q_OBJECT

public:
    static ResourceManager *getInstance ();

public:
    ResourceManager ();
    ~ResourceManager ();
				 
public:
    QIcon previous_timer_icon;
    QIcon next_timer_icon;
    QIcon previous_dish_icon;
    QIcon next_dish_icon;
    QIcon current_timer_icon;
    QIcon timer_list_icon;
    QIcon audio_enabled_icon;
    QIcon audio_disabled_icon;
    QIcon vibrosignal_enabled_icon;
    QIcon vibrosignal_disabled_icon;
    QIcon bookmarks_icon;
    QIcon reference_close_icon;
    QIcon reference_return_icon;
    QIcon reference_search_icon;
    QImage background_image;
    QImage button_stick_image;
    QSvgRenderer audio_enabled_svg;
    QSvgRenderer audio_enabled_pressed_svg;
    QSvgRenderer audio_disabled_svg;
    QSvgRenderer audio_disabled_pressed_svg;
    QString logo_filename;
};

#endif
