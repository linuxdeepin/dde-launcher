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

#ifndef MINICATEGORYWIDGET_H
#define MINICATEGORYWIDGET_H

#include <QWidget>
#include <QPushButton>

#include "model/appslistmodel.h"

class MiniCategoryItem : public QPushButton
{
    Q_OBJECT

public:
    explicit MiniCategoryItem(const QString &title, QWidget *parent = Q_NULLPTR);
};

class MiniCategoryWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(bool active READ active NOTIFY activeChanged)

public:
    explicit MiniCategoryWidget(QWidget *parent = 0);

    inline bool active() const { return m_active; }
    inline AppsListModel::AppCategory currentCategory() const { return m_currentCategory; }

signals:
    void requestRight() const;
    void activeChanged(bool) const;
    void requestCategory(const AppsListModel::AppCategory &category) const;

protected:
    void mousePressEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void enterEvent(QEvent *e);
    bool event(QEvent *event);

private slots:
    void selectNext();
    void selectPrev();

    void onCategoryListChanged();

private:
    bool m_active;
    AppsListModel::AppCategory m_currentCategory;
    AppsManager *m_appsManager;

    QButtonGroup *m_buttonGroup;

    MiniCategoryItem *m_allApps;
    MiniCategoryItem *m_internet;
    MiniCategoryItem *m_chat;
    MiniCategoryItem *m_music;
    MiniCategoryItem *m_video;
    MiniCategoryItem *m_graphics;
    MiniCategoryItem *m_game;
    MiniCategoryItem *m_office;
    MiniCategoryItem *m_reading;
    MiniCategoryItem *m_development;
    MiniCategoryItem *m_system;
    MiniCategoryItem *m_others;

    QList<MiniCategoryItem *> m_items;
};

#endif // MINICATEGORYWIDGET_H
