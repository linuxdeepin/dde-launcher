#include <gtest/gtest.h>
#include <QPixmap>

#include "../src/global_util/util.h"


class Tst_Util : public testing::Test
{
public:
    void SetUp() override
    {
        auto temp = loadSvg(m_path,20);
        temp = loadSvg(m_path,m_size);
        renderSVG(m_path,m_size);
        getQssFromFile(m_path);
        joinPath(m_path, m_path);
        getThumbnailsPath();
    }

    void TearDown() override
    {

    }

public:
    const QString m_path = "";
    const QSize m_size{50,50};
};

TEST_F(Tst_Util, util)
{

}
