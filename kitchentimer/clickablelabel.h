#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>

class ClickableLabel: public QLabel
{
    Q_OBJECT

public:
    ClickableLabel (QWidget*);
    ~ClickableLabel ();

protected:
    void mouseMoveEvent (QMouseEvent*);
    void mousePressEvent (QMouseEvent*);
    void mouseReleaseEvent (QMouseEvent*);
};
#endif
