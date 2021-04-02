#include "gtest/gtest.h"

#include <QTest>

#include "searchwidget.h"

class Tst_SearchWidget : public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

public:
    SearchWidget* m_widget;
};

/**
 * @brief Tst_SearchWidget::SetUp create the definition here(outside of the class) can solve
 * the warning message: "** has no out-of-line definition..."
 *
 */
void Tst_SearchWidget::SetUp()
{
    m_widget = new SearchWidget;
}

void Tst_SearchWidget::TearDown()
{
    if (m_widget) {
        delete  m_widget;
        m_widget = nullptr;
    }
}

TEST_F(Tst_SearchWidget, showToggle_test)
{
    m_widget->showToggle();

    QCOMPARE(m_widget->categoryBtn()->isVisible(), true);
    QCOMPARE(m_widget->toggleModeBtn()->isVisible(), true);
}

TEST_F(Tst_SearchWidget, hideToggle_test)
{
    m_widget->hideToggle();

    QCOMPARE(!m_widget->categoryBtn()->isVisible(), true);
    QCOMPARE(!m_widget->toggleModeBtn()->isVisible(), true);
}

