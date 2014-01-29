#ifndef CUSTOMDIAL_H
#define CUSTOMDIAL_H

#include <QDial>

class CustomDial: public QAbstractSlider
{
    Q_OBJECT

public:
    CustomDial (QWidget*);
    ~CustomDial ();
    void paintEvent (QPaintEvent*);

protected:
    void mouseMoveEvent (QMouseEvent*);
    void mousePressEvent (QMouseEvent*);
    void mouseReleaseEvent (QMouseEvent*);

private:
    void getValuableGeometry (QPoint&, QSize&);
    void setValueByPos (const QPoint&, const QSize&);
};
#endif
