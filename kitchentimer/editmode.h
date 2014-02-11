// -*- mode: c++ -*-

#ifndef EDITMODE_H
#define EDITMODE_H

#include <QTimer>


class EditMode: public QObject
{
    Q_OBJECT

public:
    EditMode ();
    bool isEnabled ();
    bool isBlocked ();
    bool isHalted ();
    bool isHaltedByTimeout ();
    void halt ();
    void unhalt ();
    void enter (int);
    void enterPressed (int, int);
    void leave ();

private slots:
    void unblockEdit ();
    void unhaltByTimeout ();

private:
    bool is_enabled;
    bool is_blocked;
    bool halted;
    bool halted_by_timeout;
    QTimer unblock_timer;
    QTimer unhalt_timer;

signals:
    void unblockedByTimeout ();
    void unhaltedByTimeout ();
};

#endif
