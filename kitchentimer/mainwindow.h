// -*- mode: c++ -*-

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QStackedWidget>

class PageIntro;
class PageTimers;
class PageDishSelect;
class PageSettings;


class MainWindow: public QStackedWidget
{
    Q_OBJECT

public:
    MainWindow ();
    ~MainWindow ();

public slots:
    void switchToPageTimers ();
    void switchToPageDishSelect ();
    void switchToPageSettings ();
    void setCurrentDish (int);
    void leavePageDishSelect ();
    void cancelCurrentTimer ();
    void acceptCurrentTimer (const QString&, const QTime&);
    void stopCurrentTimer ();
    void setStartCurrentTimer (const QTime&);
    void soundAnalogTimerPressed ();
    void soundAnalogTimerReleased ();
    void soundAnalogTimerSlide ();
    void signalLongPress ();
    void signalManualAlarm ();
    void previousDish ();
    void nextDish ();
    void showAbove ();

private:
    PageIntro *page_intro;
    PageTimers *page_timers;
    PageDishSelect *page_dish_select;
    PageSettings *page_settings;
};

#endif
