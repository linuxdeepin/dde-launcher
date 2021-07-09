#include "minicategorywidget.h"

#include <QTest>

#include "gtest/gtest.h"

class Tst_MiniCategoryItem : public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

public:
    MiniCategoryItem *m_miniCategoryItem;
};

void Tst_MiniCategoryItem::SetUp()
{
    m_miniCategoryItem = new MiniCategoryItem("");
}

void Tst_MiniCategoryItem::TearDown()
{
    if (m_miniCategoryItem) {
        delete m_miniCategoryItem;
        m_miniCategoryItem = nullptr;
    }
}

TEST_F(Tst_MiniCategoryItem, itemIsCheckable_test)
{
    QCOMPARE(m_miniCategoryItem->isCheckable(), true);
}
