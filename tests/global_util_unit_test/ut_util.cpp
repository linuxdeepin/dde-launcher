#include "util.h"

#include <QPixmap>
#include <QSize>

#include <gtest/gtest.h>

class Tst_Util : public testing::Test
{
};

TEST_F(Tst_Util, util)
{
    QSize size = QSize(50, 50);
    auto temp = loadSvg("", 20);
    temp = loadSvg("", size);
    renderSVG("", size);
}
