#define private public
#include "menuworker.h"
#include "appgridview.h"
#undef private

#include <QTest>
#include <QMenu>

#include <gtest/gtest.h>

class Tst_MenuWorker : public testing::Test
{};

TEST_F(Tst_MenuWorker, menuWorker_test)
{
    MenuWorker worker;
    QMenu menu;
    menu.setAccessibleName("popmenu");

    QSignalMapper *signalMapper = new QSignalMapper(&menu);
    worker.creatMenuByAppItem(&menu, signalMapper);

    for (int index = 0; index < 7; index++)
        signalMapper->mapped(index);

    menu.aboutToHide();

    const QModelIndex index;
    worker.setCurrentModelIndex(index);
    QVERIFY(index == worker.getCurrentModelIndex());

    ASSERT_FALSE(worker.isMenuShown());

    worker.handleOpen();

    worker.m_appKey = "deepin-editor";
    worker.m_isItemOnDesktop = true;
    worker.handleToDesktop();

    worker.m_isItemOnDesktop = false;
    worker.handleToDesktop();

    worker.m_isItemOnDock = true;
    worker.handleToDock();

    worker.m_isItemOnDock = false;
    worker.handleToDock();

    worker.handleToProxy();
    worker.handleSwitchScaling();

    for (int i = 1; i < 7; i++)
        worker.handleMenuAction(MenuWorker::MenuAction(i));

    worker.handleMenuClosed();
}
