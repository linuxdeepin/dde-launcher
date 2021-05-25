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

#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QLineEdit>
#include <QLabel>
#include <QSpacerItem>
#include <QPropertyAnimation>
#include <QProxyStyle>

#include <DIconButton>

DWIDGET_USE_NAMESPACE

class SearchLineeditStyle : public QProxyStyle
{
    friend class SearchLineEdit;

public:

    SearchLineeditStyle(QStyle *style = nullptr);

    int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const;

private:
    bool hideCursor;
};

class SearchLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit SearchLineEdit(QWidget *parent = nullptr);
    ~ SearchLineEdit() override;

public slots:
    void normalMode();
    void moveFloatWidget();

private slots:
    void editMode();
    void onTextChanged();

protected:
    bool event(QEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void themeChanged();

private:
    DIconButton *m_icon;
    DIconButton *m_clear;
    QLabel *m_placeholderText;
    QWidget *m_floatWidget;
#ifndef ARCH_MIPSEL
    QPropertyAnimation *m_floatAni;
#endif
    SearchLineeditStyle *m_editStyle;
};

#endif // SEARCHLINEEDIT_H
