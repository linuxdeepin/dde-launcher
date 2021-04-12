#include <gtest/gtest.h>
#include <QPixmap>
#include <QSize>

#include "util.h"


class Tst_Util : public testing::Test
{
public:
};

TEST_F(Tst_Util, util)
{
    QSize size = QSize(50, 50);
    auto temp = loadSvg("", 20);
    temp = loadSvg("", size);
    renderSVG("", size);
}
