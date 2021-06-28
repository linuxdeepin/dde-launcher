#define private public
#include "menuworker.h"
#undef private

#include <QTest>
#include <QMenu>

#include <gtest/gtest.h>

class Tst_MenuWorker : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new MenuWorker();
    }

    void TearDown() override
    {
        if (m_widget) {
            delete m_widget;
            m_widget = nullptr;
        }
    }

public:
    MenuWorker *m_widget;
};

TEST_F(Tst_MenuWorker, menuWorker_test)
{
    QMenu *menu = new QMenu;
    menu->setAccessibleName("popmenu");

    QSignalMapper *signalMapper = new QSignalMapper(menu);
    m_widget->creatMenuByAppItem(menu, signalMapper);

    for (int index = 0; index < 7; index++)
        signalMapper->mapped(1);

    menu->aboutToHide();

    delete menu;
    menu = nullptr;

    const QModelIndex index;
    m_widget->setCurrentModelIndex(index);
    QVERIFY(index == m_widget->getCurrentModelIndex());
}
