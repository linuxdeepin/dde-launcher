#ifndef MINIFRAME_H
#define MINIFRAME_H

#include "launcherinterface.h"

#include <QWidget>
#include <DBlurEffectWidget>

DWIDGET_USE_NAMESPACE

class DBusDock;
class MiniFrame : public DBlurEffectWidget, public LauncherInterface
{
    Q_OBJECT

public:
    explicit MiniFrame(QWidget *parent = 0);

private:
    void showLauncher();
    void hideLauncher();
    bool visible();

    void mousePressEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void showEvent(QShowEvent *e);

private slots:
    void adjustPosition();

private:
    DBusDock *m_dockInter;
};

#endif // MINIFRAME_H
