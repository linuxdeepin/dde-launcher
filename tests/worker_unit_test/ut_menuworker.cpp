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
    qDebug() << "menu worker's rect: " << worker.menuGeometry();

    worker.handleOpen();
    worker.handleToDesktop();
    worker.handleToDock();
    worker.handleToStartup();
    worker.handleToProxy();
    worker.handleSwitchScaling();
    worker.handleMenuClosed();
}
