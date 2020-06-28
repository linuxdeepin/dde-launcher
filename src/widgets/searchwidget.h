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
#include "src/global_util/calculate_util.h"
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

    void setLeftSpacing(int spacing);
    void setRightSpacing(int spacing);

    void showToggle();
    void hideToggle();

    void updateSize(double scaleX, double scaleY);
public slots:
    void clearSearchContent();

signals:
    void searchTextChanged(const QString &text) const;
    void toggleMode();

private:
    DSearchEdit *m_searchEdit;
    QFrame *m_leftSpacing;
    QFrame *m_rightSpacing;
    DFloatingButton *m_toggleCategoryBtn;
    DFloatingButton *m_toggleModeBtn;
    CalculateUtil* m_calcUtil;
};

#endif // SEARCHWIDGET_H
