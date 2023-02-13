/*
 * Copyright (C) 2021 ~ 2022 Deepin Technology Co., Ltd.
 *
 * Author:     songwentao <songwentao@uniontech.com>
 *
 * Maintainer: songwentao <songwentao@uniontech.com>
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
 *
 * This program aims to cache the the icon and name of apps to the hash table,
 * which can decrease the repeated resource consumption of loading the app info in the
 * running time.
 */

#ifndef EDITLABEL_H
#define EDITLABEL_H

#include <dtkwidget_global.h>
#include <DFontSizeManager>

#include <QWidget>
#include <QLineEdit>
#include <QLabel>

class EditLabel : public QWidget
{
    Q_OBJECT

public:
    explicit EditLabel(QWidget *parent = nullptr);
    ~EditLabel() override;

    void setText(int maxWidth, const QString &title = QString());
    QString text() const;

    void cancelEditState();

signals:
    void titleChanged();

private slots:
    void onReturnPressed();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void initUi();
    void initConnection();
    void initAccessibleName();

private:
    QLineEdit *m_lineEdit;
    QLabel *m_label;
    QString m_oldTitle;      // 修改前的标题名称
    QString m_originTitle;   // 没有添加省略号的标题名称(输入的原始标题内容)
    int m_maxWidth;          // 文本最大宽度
};

#endif // EDITLABEL_H
