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

#ifndef CATEGORYBUTTON_H
#define CATEGORYBUTTON_H

#include "../model/appslistmodel.h"
#include "global_util/calculate_util.h"

#include <QAbstractButton>
#include <QLabel>

class CategoryButton : public QAbstractButton
{
    Q_OBJECT

public:
    enum State {
        Normal,
        Hover,
        Checked,
    };

public:
    explicit CategoryButton(const AppsListModel::AppCategory category, QWidget *parent = 0);

    Q_PROPERTY(qreal titleOpacity READ titleOpacity WRITE setTitleOpacity)
    Q_PROPERTY(qreal zoomLevel READ zoomLevel WRITE setZoomLevel)

    QLabel *textLabel();

    qreal titleOpacity() const;
    void setTitleOpacity(const qreal &titleOpacity);

    qreal zoomLevel() const;
    void setZoomLevel(const qreal &zoomLevel);

public slots:
    void setChecked(bool isChecked);
    void setTextVisible(bool visible, const bool animation = false);
    AppsListModel::AppCategory category() const;

protected:
    void paintEvent(QPaintEvent *e);
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);

private:
    void setInfoByCategory();
    void updateState(const State state);
    void updateTextColor();
    void addTextShadow();

private slots:
    void relayout();

private:
    CalculateUtil *m_calcUtil;
    State m_state = Checked;
    AppsListModel::AppCategory m_category;
    QString m_iconName;

    QLabel *m_iconLabel;
    QLabel *m_textLabel;

    QPropertyAnimation *m_opacityAnimation;
    qreal m_titleOpacity = 1;

    qreal m_zoomLevel = 1;
};

#endif // CATEGORYBUTTON_H
