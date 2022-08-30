// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include "categorybutton.h"
#include "calculate_util.h"

#include <QFrame>
#include <QButtonGroup>

class NavigationWidget : public QFrame
{
    Q_OBJECT

public:
    explicit NavigationWidget(QWidget *parent = nullptr);
    ~NavigationWidget() {}

    CategoryButton *button(const AppsListModel::AppCategory category) const;

    void updateSize();
    QButtonGroup * buttonGroup() { return m_categoryGroup;}
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
