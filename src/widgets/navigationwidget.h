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

#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include "categorybutton.h"
#include "src/global_util/calculate_util.h"

#include <QFrame>
#include <QButtonGroup>

class NavigationWidget : public QFrame
{
    Q_OBJECT

public:
    explicit NavigationWidget(QWidget *parent = nullptr);

    void setButtonsVisible(const bool visible);

    CategoryButton *button(const AppsListModel::AppCategory category) const;

    void updateSize();
signals:
    void scrollToCategory(const AppsListModel::AppCategory oldCategory, const AppsListModel::AppCategory newCategory) const;
    void mouseEntered();

public slots:
    void setCurrentCategory(const AppsListModel::AppCategory category);
    void setCancelCurrentCategory(const AppsListModel::AppCategory category);
    void refershCategoryVisible(const AppsListModel::AppCategory category, const bool visible);

protected:
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void showEvent(QShowEvent *e);

private:
    void initUI();
    void initConnection();

    void buttonClicked();

private:
    CalculateUtil *m_calcUtil;
    QButtonGroup *m_categoryGroup;
    CategoryButton *m_internetBtn;
    CategoryButton *m_chatBtn;
    CategoryButton *m_musicBtn;
    CategoryButton *m_videoBtn;
    CategoryButton *m_graphicsBtn;
    CategoryButton *m_gameBtn;
    CategoryButton *m_officeBtn;
    CategoryButton *m_readingBtn;
    CategoryButton *m_developmentBtn;
    CategoryButton *m_systemBtn;
    CategoryButton *m_othersBtn;
    AppsListModel::AppCategory m_currentCategory;
};

#endif // NAVIGATIONWIDGET_H
