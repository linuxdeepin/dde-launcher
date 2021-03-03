/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     shaojun <wangshaojun_cm@deepin.com>
 *
 * Maintainer: shaojun <wangshaojun_cm@deepin.com>
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

#include "pagecontrol.h"
#include "util.h"

#include <QBoxLayout>

pageControl::pageControl(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(PAGE_ICON_SPACE);
    setLayout(layout);

    m_iconActive = loadSvg(":/widgets/images/page_indicator_active.svg",PAGE_ICON_SIZE);
    m_iconNormal = loadSvg(":/widgets/images/page_indicator.svg",PAGE_ICON_SIZE);
}

void pageControl::setPageCount(int count)
{
    for (int i = m_pageCount ; i < count ; i++)
        addButton();

    for (int i = count; i < m_pageCount ; i++) {
        DIconButton *pageButton = qobject_cast<DIconButton *>(layout()->takeAt(0)->widget());
        pageButton->deleteLater();
    }

    m_pageCount = count;

    setCurrent(0);
}

void pageControl::setCurrent(int pageIndex)
{
    if (pageIndex < layout()->count()) {
        DIconButton *pageButton = qobject_cast<DIconButton *>(layout()->itemAt(pageIndex)->widget());
        pageButton->setChecked(true);
    }
}

void pageControl::UpdateIconSize(double scaleX, double scaleY)
{
    double scale = (qAbs(1 - scaleX) < qAbs(1 - scaleY)) ? scaleX : scaleY;
    for (int i = 0; i < m_pageCount ; i++) {
        DIconButton *pageButton = qobject_cast<DIconButton *>(layout()->itemAt(i)->widget());
        pageButton->setIconSize(QSize(PAGE_ICON_SIZE * scale, PAGE_ICON_SIZE * scale));
        pageButton->setFixedSize(QSize(PAGE_BUTTON_SIZE * scale, PAGE_BUTTON_SIZE * scale));
    }
    setFixedHeight(PAGE_BUTTON_SIZE * scale);
}

void pageControl::addButton()
{
    DIconButton *pageButton = new DIconButton(this);
    pageButton->setIcon(m_iconNormal);
    pageButton->setAccessibleName("thisPageButton");
    pageButton->setIconSize(QSize(PAGE_ICON_SIZE, PAGE_ICON_SIZE));
    pageButton->setFixedSize(QSize(PAGE_BUTTON_SIZE, PAGE_BUTTON_SIZE));
    pageButton->setBackgroundRole(DPalette::Button);
    pageButton->setAutoExclusive(true);
    pageButton->setFocusPolicy(Qt::NoFocus);
    pageButton->setCheckable(true);
    pageButton->setFlat(true);

    layout()->addWidget(pageButton);

    connect(pageButton, &DIconButton::toggled, this, &pageControl::pageBtnClicked);
}

void pageControl::pageBtnClicked(bool checked)
{
    DIconButton *pageButton = qobject_cast<DIconButton *>(sender());
    if (!pageButton)
        return;

    pageButton->setAccessibleName("addPageBtn");
    pageButton->setIcon(checked ?  m_iconActive : m_iconNormal);

    if (checked)
        emit onPageChanged(layout()->indexOf(pageButton));
}
