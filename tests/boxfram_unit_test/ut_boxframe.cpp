// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "boxframe.h"
#undef private

#include <gtest/gtest.h>

class Tst_Boxframe : public testing::Test
{};

TEST_F(Tst_Boxframe, checkBackground_test)
{
    BoxFrame frame;
    frame.setBackground(frame.m_defaultBg);
    frame.setBlurBackground(frame.m_defaultBg);

    // 清空上一次的背景
    frame.m_lastUrl.clear();
    frame.m_lastBlurUrl.clear();
}
