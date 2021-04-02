#include "gtest/gtest.h"

#include <QTest>
#include <QSignalSpy>

#include "navigationwidget.h"
#include "appslistmodel.h"

class Tst_NavigationWidget : public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

public:
    NavigationWidget* m_widget;
};

void Tst_NavigationWidget::SetUp()
{
    m_widget = new NavigationWidget;
}

void Tst_NavigationWidget::TearDown()
{
    if (m_widget) {
        delete m_widget;
        m_widget = nullptr;
    }
}


TEST_F(Tst_NavigationWidget, categoryButton_test)
{
    EXPECT_NE(m_widget->button(AppsListModel::Internet), nullptr);
    EXPECT_NE(m_widget->button(AppsListModel::Chat), nullptr);
    EXPECT_NE(m_widget->button(AppsListModel::Music), nullptr);
    EXPECT_NE(m_widget->button(AppsListModel::Video), nullptr);
    EXPECT_NE(m_widget->button(AppsListModel::Graphics), nullptr);
    EXPECT_NE(m_widget->button(AppsListModel::Game), nullptr);
    EXPECT_NE(m_widget->button(AppsListModel::Office), nullptr);
    EXPECT_NE(m_widget->button(AppsListModel::Reading), nullptr);
    EXPECT_NE(m_widget->button(AppsListModel::Development), nullptr);
    EXPECT_NE(m_widget->button(AppsListModel::System), nullptr);
    EXPECT_NE(m_widget->button(AppsListModel::Others), nullptr);
}

TEST_F(Tst_NavigationWidget, enterEvent_test)
{
    QEvent event(QEvent::Enter);
    QApplication::sendEvent(m_widget, &event);

    QTest::qWait(10);

    QSignalSpy spy(m_widget, SIGNAL(mouseEntered()));

    QCOMPARE(spy.count(), 1);
}

