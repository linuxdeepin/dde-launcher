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
#include "src/global_util/calculate_util.h"

#include <QAbstractButton>
#include <QIcon>

class CategoryButton : public QAbstractButton
{
    Q_OBJECT

public:
    enum State {
        Normal,
        Hover,
        Checked,
        Press,
    };

public:
    explicit CategoryButton(const AppsListModel::AppCategory category, QWidget *parent = nullptr);

public slots:
    void setChecked(bool isChecked);
    void setScreenScale(double fScale);
    AppsListModel::AppCategory category() const;

protected:
    void paintEvent(QPaintEvent *event);
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    void setInfoByCategory();
    void updateState(const State state);

private:
    CalculateUtil *m_calcUtil;
    State m_state = Normal;
    AppsListModel::AppCategory m_category;
    QString m_iconName;
    QString m_systemTheme;
    QPixmap m_icon;
    double m_fScale;
};

#endif // CATEGORYBUTTON_H
