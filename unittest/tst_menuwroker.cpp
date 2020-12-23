#include <gtest/gtest.h>
#include <QPixmap>

#include "../src/worker/menuworker.h"


class Tst_MenuWorker : public testing::Test
{
public:
    void SetUp() override
    {
        m_menuWorker = new MenuWorker();

    }

    void TearDown() override
    {
        delete m_menuWorker;
        m_menuWorker = nullptr;
    }

public:
    MenuWorker* m_menuWorker;
};

TEST_F(Tst_MenuWorker, util)
{
    m_menuWorker->handleMenuAction(1);
    //m_menuWorker->isItemOnDock("dde-control-center");
    //m_menuWorker->handleToDesktop();
}
