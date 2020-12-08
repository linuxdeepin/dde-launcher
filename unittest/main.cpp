#include <QApplication>
#include <QDebug>

#include <DLog>

#include <gtest/gtest.h>

DCORE_USE_NAMESPACE
int main(int argc, char **argv)
{
    // gerrit编译时没有显示器，需要指定环境变量
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc,argv);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    qDebug() << "start dde-clipboard test cases ..............";
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    qDebug() << "end dde-clipboard test cases ..............";
    return ret;
}
