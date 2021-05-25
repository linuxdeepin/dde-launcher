#include <gtest/gtest.h>

#ifdef QT_DEBUG
#include <sanitizer/asan_interface.h>
#endif

#include <DLog>

#include <QDebug>
#include <QApplication>

DCORE_USE_NAMESPACE

int main(int argc, char **argv)
{
    // gerrit编译时没有显示器，需要指定环境变量
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc,argv);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    qDebug() << "start dde-launcher test cases ..............";
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    qDebug() << "end dde-launcher test cases ..............";

#ifdef QT_DEBUG
    __sanitizer_set_report_path("asan.log");
#endif
    return ret;
}
