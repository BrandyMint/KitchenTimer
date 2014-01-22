// -*- mode: c++ -*-

#ifndef PAGEINTRO_H
#define PAGEINTRO_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QMouseEvent;
class QTimer;
QT_END_NAMESPACE

class PageIntro: public QWidget
{
    Q_OBJECT

public:
    PageIntro (QWidget*);
    ~PageIntro ();

protected:
    void mouseReleaseEvent (QMouseEvent*);

private slots:
    void skipIntro ();

private:
    QTimer *skip_intro_timer;

signals:
    void switchToPageTimer ();
};

#endif
