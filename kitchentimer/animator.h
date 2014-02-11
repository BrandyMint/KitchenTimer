#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE


class Animator: public QObject
{
    Q_OBJECT

public:
    Animator (QWidget*, int);
    bool isRunning ();
    double phase ();
    void setMinFrameTimeout (int);

public slots:
    void start (int = -1);
    void stop ();

private slots:
    void iterateUpdate ();

private:
    QWidget *widget;
    int min_frame_timeout;
    QTimer timer;
    bool running;
    QElapsedTimer elapsed_timer;
    int duration_ms;

signals:
    void stopped ();
};

#endif
