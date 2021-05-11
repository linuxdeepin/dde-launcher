#include <gtest/gtest.h>

#define private public
#include "launchersys.h"
#include "iconfreshthread.h"
#undef private

class Tst_LauncherSys : public testing::Test
{
public:
    void SetUp() override
    {
        m_launcherSys = new LauncherSys(nullptr);
    }

    void TearDown() override
    {
        if (m_launcherSys->m_appIconFreshThread->isRunning())
            m_launcherSys->m_appIconFreshThread->releaseThread();

        if (m_launcherSys) {
            delete m_launcherSys;
            m_launcherSys = nullptr;
        }
    }

public:
    LauncherSys* m_launcherSys;
};

TEST_F(Tst_LauncherSys, launcherSys_test)
{
    m_launcherSys->registerRegion();
    m_launcherSys->displayModeChanged();

    m_launcherSys->onDisplayModeChanged();
    m_launcherSys->onVisibleChanged();

    m_launcherSys->hideLauncher();
    m_launcherSys->unRegisterRegion();
}
