#ifndef SHAREDEVENTFILTER_H
#define SHAREDEVENTFILTER_H

#include <QObject>
#include <QKeyEvent>

#include "launcherinterface.h"

class SharedEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit SharedEventFilter(QObject *parent = 0);

    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    LauncherInterface *m_frame;

    bool handleKeyEvent(QKeyEvent *event);
};

#endif // SHAREDEVENTFILTER_H
