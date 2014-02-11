// -*- mode: c++ -*-

#ifndef TIMER_H
#define TIMER_H

#include <QObject>
#include <QTime>
#include <QTimer>
#include <QElapsedTimer>


class Timer: public QObject
{
    Q_OBJECT

public:
    Timer (const QTime&, const QString&);
    const QTime &getPeriod ();
    void setTimeLeft (const QTime&);
    QTime getTimeLeft ();
    void start ();
    void stop ();
    bool isRunning ();
    void setTitle (const QString&);
    const QString &getTitle ();

private slots:
    void internalTick ();
    void internalTimeout ();

private:
    QTime period;
    QTime time_left;
    QString title;
    bool running;
    QTimer main_timer;
    QTimer ticker;
    QElapsedTimer elapsed_timer;

signals:
    void updateTick ();
    void timeout ();
    void newTimeSet ();
};

#endif
