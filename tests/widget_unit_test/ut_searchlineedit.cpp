#define private public
#include "searchlineedit.h"
#undef private

#include <gtest/gtest.h>

class Tst_Searchlineedit : public testing::Test
{};

TEST_F(Tst_Searchlineedit, searchLineEdit_test)
{
    SearchLineEdit edit;
    edit.editMode();
    edit.onTextChanged();
    edit.moveFloatWidget();
}
