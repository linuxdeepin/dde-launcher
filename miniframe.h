#ifndef MINIFRAME_H
#define MINIFRAME_H

#include "launcherinterface.h"

#include <QWidget>
#include <DBlurEffectWidget>
#include <dimagebutton.h>

DWIDGET_USE_NAMESPACE

class DBusDock;
class MiniFrameNavigation;
class SearchLineEdit;
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

    DImageButton *m_viewToggle;
    DImageButton *m_modeToggle;
    SearchLineEdit *m_searchEdit;

    MiniFrameNavigation *m_navigation;
    QWidget *m_viewWrapper;
};

#endif // MINIFRAME_H
