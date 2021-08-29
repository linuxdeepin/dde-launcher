//#include "appslistmodel.h"

//#define private public
//#include "fullscreenframe.h"
//#include "multipagesview.h"
//#undef private

//#include <gtest/gtest.h>

//class Tst_Fullscreenframe : public testing::Test
//{
//};

//TEST_F(Tst_Fullscreenframe, pressEvent_test)
//{
//    FullScreenFrame fullscreen;
//    QMouseEvent leftPressEvent(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
//    fullscreen.mousePressEvent(&leftPressEvent);

//    QMouseEvent rightPressEvent(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::RightButton, Qt::RightButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
//    fullscreen.mousePressEvent(&rightPressEvent);
//}

//TEST_F(Tst_Fullscreenframe, moveEvent_test)
//{
//    FullScreenFrame fullscreen;

//    // m_mouse_press未点击时
//    fullscreen.m_mouse_press = false;
//    fullscreen.m_animationGroup->start();
//    QMouseEvent moveEvent(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//    fullscreen.mouseMoveEvent(&moveEvent);

//    // m_mouse_press点击时
//    fullscreen.m_mouse_press = true;
//    fullscreen.m_animationGroup->stop();
//    QMouseEvent moveEvent_(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//    fullscreen.mouseMoveEvent(&moveEvent_);
//}

//TEST_F(Tst_Fullscreenframe, releaseEvent_test)
//{
//    FullScreenFrame fullscreen;

//    // 普通模式时
//    QMouseEvent normalReleaseEvent(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
//    fullscreen.mouseReleaseEvent(&normalReleaseEvent);

//    // m_mouse_press点击时
//    fullscreen.m_mouse_press = false;
//    QMouseEvent pressReleaseEvent(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
//    fullscreen.mouseReleaseEvent(&pressReleaseEvent);

//    // 分类模式时
//    fullscreen.m_displayMode = GROUP_BY_CATEGORY;
//    QMouseEvent releaseEvent_(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//    fullscreen.mouseReleaseEvent(&releaseEvent_);
//}

//TEST_F(Tst_Fullscreenframe, wheelEvent_test)
//{
//    FullScreenFrame fullscreen;

//    fullscreen.m_displayMode = GROUP_BY_CATEGORY;
//    QWheelEvent wheelEvent(QPointF(0, 0), 0, Qt::MiddleButton, Qt::ControlModifier);
//    fullscreen.wheelEvent(&wheelEvent);

//    fullscreen.m_displayMode = SEARCH;
//}

//TEST_F(Tst_Fullscreenframe, keyEvent_test)
//{
//    FullScreenFrame fullscreen;

//    QKeyEvent minusKeyEvent(QEvent::KeyPress, Qt::Key_Minus, Qt::NoModifier, QString(""));
//    fullscreen.keyPressEvent(&minusKeyEvent);

//    QKeyEvent ctrlMinusKeyEvent(QEvent::KeyPress, Qt::Key_Minus, Qt::ControlModifier, QString(""));
//    fullscreen.keyPressEvent(&ctrlMinusKeyEvent);

//    QKeyEvent equalKeyEvent(QEvent::KeyPress, Qt::Key_Equal, Qt::NoModifier, QString(""));
//    fullscreen.keyPressEvent(&equalKeyEvent);

//    QKeyEvent controlVKeyEvent(QEvent::KeyPress, Qt::Key_Equal, Qt::ControlModifier, QString(""));
//    fullscreen.keyPressEvent(&controlVKeyEvent);
//}

//TEST_F(Tst_Fullscreenframe, showAndHideEvent_test)
//{
//    // 涉及xcb模块代码，会导致crash
//    //    FullScreenFrame fullscreen;
//    //    fullscreen.showLauncher();

//    //    fullscreen.hideLauncher();
//}

//TEST_F(Tst_Fullscreenframe, scrollToCategory_scrollCurrent_test)
//{
//    FullScreenFrame fullscreen;
//    fullscreen.scrollToCategory(AppsListModel::AppCategory::Chat, AppsListModel::AppCategory::Chat);
//}

//TEST_F(Tst_Fullscreenframe, scrollToCategory_scrollPrev_test)
//{
////    FullScreenFrame fullscreen;
////    fullscreen.scrollToCategory(AppsListModel::AppCategory::Chat, AppsListModel::AppCategory::Internet);
//}

//TEST_F(Tst_Fullscreenframe, scrollToCategory_scrollPrev1_test)
//{
////    FullScreenFrame fullscreen;
////    fullscreen.scrollToCategory(AppsListModel::AppCategory::Video, AppsListModel::AppCategory::Internet);
//}

//TEST_F(Tst_Fullscreenframe, scrollToCategory_scrollNext_test)
//{
////    FullScreenFrame fullscreen;
////    fullscreen.scrollToCategory(AppsListModel::AppCategory::Internet, AppsListModel::AppCategory::Chat);
//}

//TEST_F(Tst_Fullscreenframe, scrollToCategory_scrollNext1_test)
//{
////    FullScreenFrame fullscreen;
////    fullscreen.m_animationGroup->start();
////    fullscreen.scrollToCategory(AppsListModel::AppCategory::All, AppsListModel::AppCategory::Chat);

////    fullscreen.m_animationGroup->stop();
////    fullscreen.scrollToCategory(AppsListModel::AppCategory::All, AppsListModel::AppCategory::Chat);
//}

//TEST_F(Tst_Fullscreenframe, scrollBlurBoxWidget_test)
//{
//    BlurBoxWidget blueBox(AppsListModel::AppCategory::Internet, nullptr);
//    FullScreenFrame fullscreen;

//    ScrollWidgetAgent scrollWidgetAgent(&fullscreen);
//    scrollWidgetAgent.setPosType(Pos_M);
//    scrollWidgetAgent.setBlurBoxWidget(&blueBox);

//    fullscreen.scrollBlurBoxWidget(nullptr);
//    fullscreen.scrollBlurBoxWidget(&scrollWidgetAgent);
//}

//TEST_F(Tst_Fullscreenframe, tips_test)
//{
//    FullScreenFrame fullscreen;
//    for (int type = 0; type < 2; type++) {
//         fullscreen.updateDisplayMode(type);
//         fullscreen.showTips("");
//         fullscreen.hideTips();
//    }
//}

//TEST_F(Tst_Fullscreenframe, backtab_nextTabWidget_test)
//{
//    FullScreenFrame fullscreen;
//    fullscreen.updateDisplayMode(GROUP_BY_CATEGORY);
//    fullscreen.nextTabWidget(Qt::Key_Backtab);
//    fullscreen.updateDisplayMode(SEARCH);
//    fullscreen.nextTabWidget(Qt::Key_Backtab);
//}

//TEST_F(Tst_Fullscreenframe, tab_nextTabWidget_test)
//{
//    FullScreenFrame fullscreen;
//    fullscreen.updateDisplayMode(GROUP_BY_CATEGORY);
//    fullscreen.nextTabWidget(Qt::Key_Tab);
//    fullscreen.updateDisplayMode(SEARCH);
//    fullscreen.nextTabWidget(Qt::Key_Tab);
//}

//TEST_F(Tst_Fullscreenframe, privateMethod_test)
//{
//    FullScreenFrame fullscreen;
//    fullscreen.primaryScreenChanged();
//    fullscreen.updateDockPosition();
//    fullscreen.updateGeometry();

//    fullscreen.updateDisplayMode(GROUP_BY_CATEGORY);
//    fullscreen.nextTabWidget(Qt::Key_Backtab);

//    fullscreen.categoryListChanged();
//    fullscreen.moveCurrentSelectApp(Qt::Key_Tab);
//    fullscreen.moveCurrentSelectApp(Qt::Key_Undo);
//    fullscreen.moveCurrentSelectApp(Qt::Key_Space);
//    fullscreen.moveCurrentSelectApp(Qt::Key_Backtab);
//    fullscreen.moveCurrentSelectApp(Qt::Key_Right);
//    fullscreen.moveCurrentSelectApp(Qt::Key_Down);

//    fullscreen.appendToSearchEdit(static_cast<const char>(-1));
//    fullscreen.appendToSearchEdit(static_cast<const char>(0));

//    fullscreen.hideLauncher();
//    fullscreen.dockPosition();
//}

//TEST_F(Tst_Fullscreenframe, parentWidget_test)
//{
//    FullScreenFrame fullscreen;
//    AppGridView *gridView = nullptr;
//    gridView = fullscreen.m_multiPagesView->m_appGridViewList[fullscreen.m_multiPagesView->m_pageIndex];

//    ASSERT_TRUE(gridView);

//    FullScreenFrame *fullscreenFrame = nullptr;
//    fullscreenFrame = (qobject_cast<FullScreenFrame*>)(
//                gridView->parentWidget()->parentWidget()->parentWidget()->parentWidget()
//                ->parentWidget()->parentWidget()->parentWidget());

//    ASSERT_TRUE(fullscreenFrame);

//    gridView->flashDrag();
//}

//TEST_F(Tst_Fullscreenframe, otherApi_test)
//{
//    FullScreenFrame fullscreen;

//    fullscreen.inputMethodQuery(Qt::ImEnabled);
//    fullscreen.inputMethodQuery(Qt::ImCursorRectangle);
//    fullscreen.inputMethodQuery(Qt::ImFont);

//    fullscreen.m_mouse_press = true;
//    fullscreen.m_animationGroup->start();
//    fullscreen.blurBoxWidgetMaskClick(AppsListModel::Category);
//    fullscreen.blurBoxWidgetMaskClick(AppsListModel::Category);
//    fullscreen.m_animationGroup->stop();
////    fullscreen.blurBoxWidgetMaskClick(AppsListModel::Category);
////    fullscreen.blurBoxWidgetMaskClick(AppsListModel::Internet);

//    fullscreen.scrollPrev();
//    fullscreen.scrollNext();

//    fullscreen.m_displayMode = SEARCH;
//    fullscreen.showTips(QString(""));

//    fullscreen.m_displayMode = GROUP_BY_CATEGORY;
//    fullscreen.showTips(QString(""));

//    fullscreen.getCategoryGridViewList(AppsListModel::Game);

//    fullscreen.getCategoryBoxWidget(AppsListModel::AppCategory(15));

//    fullscreen.checkCurrentCategoryVisible();

////    fullscreen.scrollToCategoryFinish();

//    fullscreen.getScrollWidgetAgent(PosType::Pos_None);

//    fullscreen.getCategoryBoxWidgetByPostType(PosType::Pos_LL, AppsListModel::Internet);
//    fullscreen.getCategoryBoxWidgetByPostType(PosType::Pos_RR, AppsListModel::Others);
//    fullscreen.getCategoryBoxWidgetByPostType(PosType::Pos_LL, AppsListModel::Game);

//    fullscreen.nearestCategory(AppsListModel::Others, AppsListModel::Internet);

//    fullscreen.m_searchWidget->m_searchEdit->clear();
//    fullscreen.m_searchWidget->categoryBtn()->setFocus();
//    fullscreen.launchCurrentApp();

//    fullscreen.m_searchWidget->categoryBtn()->clearFocus();
//    fullscreen.m_displayMode = SEARCH;
//    fullscreen.launchCurrentApp();

//    fullscreen.m_displayMode = GROUP_BY_CATEGORY;
//    fullscreen.launchCurrentApp();

//    fullscreen.windowDeactiveEvent();

//    QPoint point(100, 100);
//    fullscreen.regionMonitorPoint(point);

//    ASSERT_FALSE(fullscreen.visible());
//    fullscreen.moveCurrentSelectApp(Qt::TabFocus);

//    fullscreen.appendToSearchEdit(-1);
//    fullscreen.appendToSearchEdit('1');

//}

//TEST_F(Tst_Fullscreenframe, eventFilter_test)
//{
//    FullScreenFrame fullscreen;
//    qApp->installEventFilter(&fullscreen);

//    fullscreen.m_displayMode = GROUP_BY_CATEGORY;
//    QKeyEvent tabKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier, QString(""));
//    fullscreen.keyPressEvent(&tabKeyEvent);

//    QKeyEvent leftKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier, QString(""));
//    fullscreen.m_searchWidget->keyPressEvent(&leftKeyEvent);

//    QKeyEvent rightKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier, QString(""));
//    fullscreen.m_searchWidget->keyPressEvent(&rightKeyEvent);

//    QResizeEvent resizeEvent(QSize(10, 10), QSize(20, 20));
//    fullscreen.m_contentFrame->resizeEvent(&resizeEvent);
//}
