#include "gtest/gtest.h"

#include "dbusdock.h"

class Tst_DBusDock : public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

public:
    DBusDock* m_dbusdock;
};

void Tst_DBusDock::SetUp()
{
    m_dbusdock = new DBusDock;
}

void Tst_DBusDock::TearDown()
{
    if (m_dbusdock) {
        delete m_dbusdock;
        m_dbusdock = nullptr;
    }
}

TEST_F(Tst_DBusDock, callMethod_test)
{
    int display = m_dbusdock->displayMode();
    m_dbusdock->setDisplayMode(display);

    int hideMode = m_dbusdock->hideMode();
    m_dbusdock->setHideMode(hideMode);

    int hideState = m_dbusdock->hideState();
    m_dbusdock->setHideMode(hideState);
}
