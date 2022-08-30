// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CATEGORYBUTTON_H
#define CATEGORYBUTTON_H

#include "appslistmodel.h"
#include "calculate_util.h"

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
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    void setInfoByCategory();
    void updateState(const State state);

private:
    State m_state = Normal;
    AppsListModel::AppCategory m_category;
    QString m_iconName;
    QString m_systemTheme;
    QPixmap m_icon;
    double m_fScale;
};

#endif // CATEGORYBUTTON_H
