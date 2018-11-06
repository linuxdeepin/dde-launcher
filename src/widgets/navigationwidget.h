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

#include <QWidget>
#include <QButtonGroup>

class NavigationWidget : public QFrame
{
    Q_OBJECT

public:
    explicit NavigationWidget(QWidget *parent = 0);

    Q_PROPERTY(qreal zoomLevel READ zoomLevel WRITE setZoomLevel)

    void setButtonsVisible(const bool visible);
    void setCategoryTextVisible(const bool visible, const bool animation = false);
    QLabel *categoryTextLabel(const AppsListModel::AppCategory category) const;

    qreal zoomLevel() const;
    void setZoomLevel(const qreal &zoomLevel);

signals:
    void scrollToCategory(const AppsListModel::AppCategory category) const;
    void mouseEntered();

public slots:
    void setCurrentCategory(const AppsListModel::AppCategory category);
    void refershCategoryVisible(const AppsListModel::AppCategory category, const int appNums);

protected:
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;

private:
    void initUI();
    void initConnection();

    void buttonClicked();
    CategoryButton *button(const AppsListModel::AppCategory category) const;

private:
    CalculateUtil *m_calcUtil;
    QButtonGroup *m_categoryGroup;
    CategoryButton *m_internetBtn;
    CategoryButton* m_chatBtn;
    CategoryButton *m_musicBtn;
    CategoryButton *m_videoBtn;
    CategoryButton *m_graphicsBtn;
    CategoryButton *m_gameBtn;
    CategoryButton *m_officeBtn;
    CategoryButton *m_readingBtn;
    CategoryButton *m_developmentBtn;
    CategoryButton *m_systemBtn;
    CategoryButton *m_othersBtn;

    qreal m_zoomLevel = 1;
};

#endif // NAVIGATIONWIDGET_H
