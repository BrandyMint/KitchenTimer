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
    int getMSElapsed ();
    void start ();
    void stop ();
    bool isRunning ();
    void setTitle (const QString&);
    const QString &getTitle ();

private slots:
    void internalTimeout ();

private:
    QTime period;
    QTime time_left;
    QString title;
    bool running;
    QTimer main_timer;
    QElapsedTimer elapsed_timer;

signals:
    void timeout ();
    void newTimeSet ();
};

#endif
