#include "searchwidget.h"

#include <QTest>

#include "gtest/gtest.h"

class Tst_SearchWidget : public testing::Test
{};

TEST_F(Tst_SearchWidget, showToggle_test)
{
    SearchWidget widget;
    widget.showToggle();

    QCOMPARE(widget.categoryBtn()->isVisible(), true);
    QCOMPARE(widget.toggleModeBtn()->isVisible(), true);
}

TEST_F(Tst_SearchWidget, hideToggle_test)
{
    SearchWidget widget;

    widget.hideToggle();

    QCOMPARE(!widget.categoryBtn()->isVisible(), true);
    QCOMPARE(!widget.toggleModeBtn()->isVisible(), true);
}
