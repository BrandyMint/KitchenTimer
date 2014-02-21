// -*- mode: c++ -*-

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QStackedWidget>

class PageIntro;
class PageTimers;
class PageSettings;


class MainWindow: public QStackedWidget
{
    Q_OBJECT

public:
    MainWindow ();
    ~MainWindow ();

public slots:
    void switchToPageTimers ();
    void switchToPageSettings ();
    void stopCurrentTimer ();
    void setStartCurrentTimer (const QTime&);
    void soundAnalogTimerPressed ();
    void soundAnalogTimerReleased ();
    void soundAnalogTimerSlide ();
    void signalLongPress ();
    void signalManualAlarm ();
    void showAbove ();

private:
    PageIntro *page_intro;
    PageTimers *page_timers;
    PageSettings *page_settings;
};

#endif
