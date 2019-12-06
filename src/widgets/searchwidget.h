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

#include "searchlineedit.h"
#include <dimagebutton.h>
#include <QWidget>

DWIDGET_BEGIN_NAMESPACE
class DSearchEdit;
class DFloatingButton;
DWIDGET_END_NAMESPACE

class SearchWidget : public QFrame
{
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = Q_NULLPTR);

    DSearchEdit *edit();
    DFloatingButton *categoryBtn();

    void setLeftSpacing(int spacing);
    void setRightSpacing(int spacing);

    void showToggle();
    void hideToggle();

public slots:
    void clearSearchContent();

signals:
    void searchTextChanged(const QString &text) const;
    void toggleMode();

private:

    //SearchLineEdit *m_searchEdit;
    QFrame *m_leftSpacing;
    QFrame *m_rightSpacing;
    DFloatingButton *m_toggleModeBtn;
    DFloatingButton *m_toggleCategoryBtn;
    DSearchEdit* m_searchEdit;
};

#endif // SEARCHWIDGET_H
