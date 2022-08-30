// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H
#include "calculate_util.h"
#include "searchlineedit.h"
#include <QWidget>
#include <DFloatingButton>
#include <DSearchEdit>

DWIDGET_USE_NAMESPACE

class SearchWidget : public QFrame
{
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = nullptr);

    DSearchEdit *edit();
    DFloatingButton *categoryBtn();
    DFloatingButton *toggleModeBtn();

    void setLeftSpacing(int spacing);
    void setRightSpacing(int spacing);

    void showToggle();
    void hideToggle();

    void updateSize(double scaleX, double scaleY);
    void updateCurrentCategoryBtnIcon();

public slots:
    void clearSearchContent();
    void onTextChanged(const QString &str);
    void onModeClicked();
    void onToggleCategoryChanged();

signals:
    void searchTextChanged(const QString &text, bool enableUpdateMode) const;
    void toggleMode();

private:
    bool m_enableUpdateMode;
    DSearchEdit *m_searchEdit;
    QFrame *m_leftSpacing;
    QFrame *m_rightSpacing;
    DFloatingButton *m_toggleCategoryBtn;                // 所有App模式 和 分类app模式切换按钮（左上侧）
    DFloatingButton *m_toggleModeBtn;                    // 全屏模式下收起按钮(右上)
    CalculateUtil* m_calcUtil;
};

#endif // SEARCHWIDGET_H
