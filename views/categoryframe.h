/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef CATEGORYFRAME_H
#define CATEGORYFRAME_H

#include <QFrame>

class QShowEvent;
class NavigationBar;
class CategoryTableWidget;

class CategoryFrame : public QFrame
{
    Q_OBJECT
public:
    explicit CategoryFrame(QWidget *parent = 0);
    ~CategoryFrame();

    void initUI(int leftMargin, int rightMargin, int column, int itemWidth, int gridWidth, int gridHeight);
    NavigationBar* getNavigationBar();
    CategoryTableWidget* getCategoryTabelWidget();

    QRect topGradientRect() const;
    QRect bottomGradientRect() const;
    bool changeCategoryTableViewModeFlag=true;
    bool firstInit=true;
signals:
    void showed();
    void contentScrolled(int value);
public slots:
    void setCategoryFlag(bool xflag);
protected:
    void showEvent(QShowEvent *);

private:
    void initConnect();
    NavigationBar* m_navigationBar;
    CategoryTableWidget* m_categoryTableWidget;
};

#endif // CATEGORYFRAME_H
