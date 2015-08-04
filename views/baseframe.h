#ifndef BASEFRAME_H
#define BASEFRAME_H

#include <QFrame>

class QEvent;

class BaseFrame : public QFrame
{
    Q_OBJECT
public:
    explicit BaseFrame(QWidget *parent = 0);
    ~BaseFrame();

signals:
    void mouseEnterToggled(bool flag);

public slots:

protected:
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
};

#endif // BASEFRAME_H
