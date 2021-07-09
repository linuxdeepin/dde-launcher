#include "blurboxwidget.h"
#include "appslistmodel.h"

#define private public
#include "scrollwidgetagent.h"
#undef private

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>

#include <gtest/gtest.h>

class Tst_Scrollwidgetagent : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new ScrollWidgetAgent();
    }

    void TearDown() override
    {
        if (m_widget) {
            delete m_widget;
            m_widget = nullptr;
        }
    }

public:
    ScrollWidgetAgent *m_widget;
};

TEST_F(Tst_Scrollwidgetagent, scrollWidgetAgent_test)
{
    QWidget *w = new QWidget;
    m_widget->setControlWidget(w);

    BlurBoxWidget *blueBox = new BlurBoxWidget(AppsListModel::Chat, const_cast<char *>("Chat"), w);
    m_widget->setPosType(PosType::Pos_LL);
    m_widget->setBlurBoxWidget(blueBox);
    m_widget->setPosType(PosType::Pos_L);
    m_widget->setBlurBoxWidget(blueBox);
    m_widget->setPosType(PosType::Pos_M);
    m_widget->setBlurBoxWidget(blueBox);
    m_widget->setPosType(PosType::Pos_R);
    m_widget->setBlurBoxWidget(blueBox);
    m_widget->setPosType(PosType::Pos_RR);
    m_widget->setBlurBoxWidget(blueBox);
}
