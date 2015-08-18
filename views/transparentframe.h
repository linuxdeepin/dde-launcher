#ifndef TRANSPARENTFRAME_H
#define TRANSPARENTFRAME_H

#include <QFrame>
class QMouseEvent;

class TransparentFrame : public QFrame
{
    Q_OBJECT
public:
    explicit TransparentFrame(QWidget *parent = 0);
    ~TransparentFrame();

signals:

public slots:

protected:
    void mouseReleaseEvent(QMouseEvent* event);
};

#endif // TRANSPARENTFRAME_H
