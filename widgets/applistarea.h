#ifndef APPLISTAREA_H
#define APPLISTAREA_H

#include <QScrollArea>

class AppListArea : public QScrollArea
{
    Q_OBJECT

public:
    explicit AppListArea(QWidget *parent = 0);

signals:
    void mouseEntered();

protected:
    void enterEvent(QEvent *e);
};

#endif // APPLISTAREA_H
