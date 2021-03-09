#include <gtest/gtest.h>

#define private public
#include "searchlineedit.h"
#undef private

class Tst_Searchlineedit : public testing::Test
{
public:
    void SetUp() override
    {
        m_searchLineEdit = new SearchLineEdit(nullptr);
    }

    void TearDown() override
    {
        delete m_searchLineEdit;
        m_searchLineEdit = nullptr;
    }

public:
    SearchLineEdit* m_searchLineEdit;
};

TEST_F(Tst_Searchlineedit, searchLineEdit_test)
{
    m_searchLineEdit->editMode();
    m_searchLineEdit->onTextChanged();
    m_searchLineEdit->moveFloatWidget();
}
