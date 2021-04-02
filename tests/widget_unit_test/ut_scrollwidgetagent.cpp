#include <gtest/gtest.h>
#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>

#define private public
#include "scrollwidgetagent.h"
#undef private
#include "appslistmodel.h"
#include "blurboxwidget.h"


class Tst_Scrollwidgetagent : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new ScrollWidgetAgent();
    }

    void TearDown() override
    {
        if (widget) {
            delete widget;
            widget = nullptr;
        }
    }

public:
    ScrollWidgetAgent* widget = nullptr;
};

TEST_F(Tst_Scrollwidgetagent, scrollWidgetAgent_test)
{
    QWidget *w = new QWidget;
    widget->setControlWidget(w);

    BlurBoxWidget *blueBox = new BlurBoxWidget(AppsListModel::Chat, const_cast<char *>("Chat"), w);
    widget->setPosType(PosType::Pos_LL);
    widget->setBlurBoxWidget(blueBox);
    widget->setPosType(PosType::Pos_L);
    widget->setBlurBoxWidget(blueBox);
    widget->setPosType(PosType::Pos_M);
    widget->setBlurBoxWidget(blueBox);
    widget->setPosType(PosType::Pos_R);
    widget->setBlurBoxWidget(blueBox);
    widget->setPosType(PosType::Pos_RR);
    widget->setBlurBoxWidget(blueBox);
}

