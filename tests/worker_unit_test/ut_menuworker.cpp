#include <gtest/gtest.h>
#include <QtTest/QtTest>
#include <QMenu>

#define private public
#include "menuworker.h"
#undef private

class Tst_MenuWorker : public testing::Test
{
public:
    void SetUp() override
    {
        obj = new MenuWorker();

    }

    void TearDown() override
    {
        if (obj) {
            delete obj;
            obj = nullptr;
        }
    }

public:
    MenuWorker* obj;
};

TEST_F(Tst_MenuWorker, menuWorker_test)
{
    QMenu *menu = new QMenu;
    menu->setAccessibleName("popmenu");

    QSignalMapper *signalMapper = new QSignalMapper(menu);
    obj->creatMenuByAppItem(menu, signalMapper);

    for (int index = 0; index < 7; index++) {
        signalMapper->mapped(1);
    }
    menu->aboutToHide();

    const QModelIndex index;
    obj->setCurrentModelIndex(index);
    QVERIFY(index == obj->getCurrentModelIndex());
}
