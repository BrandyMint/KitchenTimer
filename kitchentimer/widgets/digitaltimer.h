#ifndef DIGITALTIMER_H
#define DIGITALTIMER_H

#include <QLabel>
#include <QTime>

class DigitalTimer: public QLabel
{
    Q_OBJECT

public:
    DigitalTimer (QWidget*);
    ~DigitalTimer ();
    void setEditMode (bool);
    void setTime (const QTime&);

protected:
    void mouseMoveEvent (QMouseEvent*);
    void mousePressEvent (QMouseEvent*);
    void mouseReleaseEvent (QMouseEvent*);

private:
    QTime value;
    bool edit_mode;
    QFont font;

signals:
    void enterEditModeRequested ();
};

#endif
