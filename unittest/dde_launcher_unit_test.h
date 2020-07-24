#ifndef LauncherUnitTest_H
#define LauncherUnitTest_H

#include <QObject>

class LauncherUnitTest : public QObject
{
    Q_OBJECT
public:
    explicit LauncherUnitTest(QObject *parent = nullptr);
    ~LauncherUnitTest();
    void initTestCase();
    void cleanupTestCase();
private slots:
    //第一个测试单元
    void case1_test();
    //第二个测试单元 测试DBusDisplay 接口
    void case2_testDisplayDBus();
    //第三个测试单元 测试DBusDock
    void case3_testDockDBus();
    //第四个测试单元 测试DBusFileInfo
    void case4_testFileInfoDBus();
    //第五个测试单元 测试DBusLauncher
    void case5_testLauncherDBus();
    //第六个测试单元 测试DBusMenu
    void case6_testMenuDBus();
    //第七个测试单元 测试DBusMenuManager
    void case7_testMenuManagerDBus();
    //第八个测试单元 测试DBusStartManager
    void case8_testStartManagerDBus();
    //第九个测试单元 测试MonitorInterface
    void case9_testMonitorInterface();
};

#endif // LauncherUnitTest_H
