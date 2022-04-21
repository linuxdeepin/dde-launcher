/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    DFloatingButton *m_toggleModeBtn;                    // 全屏模式下收起按钮(右上)
    CalculateUtil* m_calcUtil;
};

#endif // SEARCHWIDGET_H
