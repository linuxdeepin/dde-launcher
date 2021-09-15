#define private public
#include "searchlineedit.h"
#undef private

#include <QApplication>
#include <QFocusEvent>

#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE

#include <gtest/gtest.h>

class Tst_Searchlineedit : public testing::Test
{};

TEST_F(Tst_Searchlineedit, searchLineEdit_test)
{
    SearchLineEdit edit;

    edit.editMode();
    edit.onTextChanged();
    edit.moveFloatWidget();

    edit.themeChanged(DGuiApplicationHelper::instance()->themeType());

    for (int i = 1; i <= 2; i++)
        edit.themeChanged(DGuiApplicationHelper::ColorType(i));

    QStyleOption styleOption;
    QWidget widget;

    edit.m_editStyle->pixelMetric(QStyle::PM_TextCursorWidth, &styleOption, &widget);
    edit.m_editStyle->pixelMetric(QStyle::PM_TabCloseIndicatorHeight, &styleOption, &widget);
}

TEST_F(Tst_Searchlineedit, event_test)
{
    SearchLineEdit edit;

    QFocusEvent focusInEvent(QEvent::FocusIn, Qt::MouseFocusReason);
    QApplication::sendEvent(&edit, &focusInEvent);

    QFocusEvent focusOutEvent(QEvent::FocusOut, Qt::MouseFocusReason);
    QApplication::sendEvent(&edit, &focusOutEvent);

    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Undo, Qt::NoModifier);
    QApplication::sendEvent(&edit, &keyPressEvent);

    QKeyEvent keyReleaseEvent(QEvent::KeyRelease, Qt::Key_Undo, Qt::NoModifier);
    QApplication::sendEvent(&edit, &keyReleaseEvent);

    QResizeEvent resizeEvent(QSize(10, 10), QSize(10, 10));
    QApplication::sendEvent(&edit, &resizeEvent);
}
