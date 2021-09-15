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
{};

TEST_F(Tst_Scrollwidgetagent, scrollWidgetAgent_test)
{
    ScrollWidgetAgent agent;
    QWidget *w = new QWidget();
    agent.setControlWidget(w);

    BlurBoxWidget *chatBox = new BlurBoxWidget(AppsListModel::Chat, const_cast<char *>("Chat"), w);
    BlurBoxWidget *internetBox = new BlurBoxWidget(AppsListModel::Internet, const_cast<char *>("Internet"), w);
    agent.setPosType(PosType::Pos_LL);
    agent.setBlurBoxWidget(chatBox);
    agent.setPosType(PosType::Pos_L);
    agent.setBlurBoxWidget(chatBox);
    agent.setPosType(PosType::Pos_M);
    agent.setBlurBoxWidget(chatBox);
    agent.setPosType(PosType::Pos_R);
    agent.setBlurBoxWidget(chatBox);
    agent.setPosType(PosType::Pos_RR);
    agent.setBlurBoxWidget(chatBox);

    agent.setBlurBoxWidget(internetBox);
    agent.setScrollToType(PosType::Pos_None);
    agent.scrollFinished();

    agent.setPos(QPoint(100, 100));
    agent.setVisible(false);

    agent.getScrollWidgetAgentPos(PosType::Pos_RR);
    agent.getScrollWidgetAgentPos(PosType::Pos_R);
    agent.getScrollWidgetAgentPos(PosType::Pos_M);
    agent.getScrollWidgetAgentPos(PosType::Pos_L);
    agent.getScrollWidgetAgentPos(PosType::Pos_LL);
    agent.getScrollWidgetAgentPos(PosType::Pos_None);

    w->deleteLater();
}

TEST_F(Tst_Scrollwidgetagent, event_test)
{

}
