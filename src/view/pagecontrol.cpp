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
#include "appsmanager.h"

#include <QBoxLayout>

PageControl::PageControl(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(PAGE_ICON_SPACE);
    setLayout(layout);

    m_iconActive = loadSvg(":/widgets/images/page_indicator_active.svg", qRound(PAGE_ICON_SIZE * devicePixelRatioF()));
    m_iconNormal = loadSvg(":/widgets/images/page_indicator.svg", qRound(PAGE_ICON_SIZE * devicePixelRatioF()));
}

void PageControl::setPageCount(int count)
{
    qInfo() << "count:" << count << ",m_pageCount:" << m_pageCount;

    for (int i = m_pageCount ; i < count ; i++) {
        qInfo() << "index:" << layout()->indexOf(m_buttonList[i]);
        qInfo() << "count:" << layout()->count();
//        if (!layout()->count())
            addButtonToLayout(m_buttonList[i]);
//        else if (layout()->indexOf(m_buttonList[i]) != -1)
//            addButtonToLayout(m_buttonList[i]);
    }

    for (int i = count; i < m_pageCount ; i++) {
        qInfo() << "33333333";
        DIconButton *pageButton = qobject_cast<DIconButton *>(layout()->takeAt(0)->widget());
        int index = m_buttonList.indexOf(pageButton);
        if (index != -1) {
            m_buttonList[index]->setVisible(false);
        }
    }

    m_pageCount = count;

    setCurrent(0);
}

void PageControl::setCurrent(int pageIndex)
{
    qInfo() << "pageINdex:" << pageIndex << ", count:" << layout()->count();
    if (pageIndex < layout()->count()) {
        DIconButton *pageButton = qobject_cast<DIconButton *>(layout()->itemAt(pageIndex)->widget());
        if (!pageButton)
            return;

        pageButton->setChecked(true);
    }
}

void PageControl::updateIconSize(double scaleX, double scaleY)
{
    double scale = (qAbs(1 - scaleX) < qAbs(1 - scaleY)) ? scaleX : scaleY;
    for (int i = 0; i < m_pageCount ; i++) {
        DIconButton *pageButton = qobject_cast<DIconButton *>(layout()->itemAt(i)->widget());
        if (!pageButton)
            return;

        pageButton->setIconSize(QSize(PAGE_ICON_SIZE * scale, PAGE_ICON_SIZE * scale));
        pageButton->setFixedSize(QSize(PAGE_BUTTON_SIZE * scale, PAGE_BUTTON_SIZE * scale));
    }

    setFixedHeight(PAGE_BUTTON_SIZE * scale);
}

void PageControl::addButton()
{
    // 获取当前最大可能的页数(假设都在一个分类)
    int totalPage = qCeil(AppsManager::instance()->appsInfoListSize(AppsListModel::All) / 12);
    for (int i = 0; i < totalPage; i++) {
        DIconButton *pageButton = new DIconButton(this);
        pageButton->setVisible(false);
        m_buttonList.append(pageButton);
    }
}

void PageControl::addButtonToLayout(DIconButton *pageButton)
{
    pageButton->setIcon(m_iconNormal);
    pageButton->setAccessibleName("thisPageButton");
    pageButton->setIconSize(QSize(PAGE_ICON_SIZE, PAGE_ICON_SIZE));
    pageButton->setFixedSize(QSize(PAGE_BUTTON_SIZE, PAGE_BUTTON_SIZE));
    pageButton->setBackgroundRole(DPalette::Button);
    pageButton->setAutoExclusive(true);
    pageButton->setFocusPolicy(Qt::NoFocus);
    pageButton->setCheckable(true);
    pageButton->setFlat(true);
    pageButton->setVisible(true);
    layout()->addWidget(pageButton);

    connect(pageButton, &DIconButton::toggled, this, &PageControl::pageBtnClicked);
}

void PageControl::pageBtnClicked(bool checked)
{
    DIconButton *pageButton = qobject_cast<DIconButton *>(sender());
    if (!pageButton)
        return;

    pageButton->setAccessibleName("addPageBtn");
    pageButton->setIcon(checked ?  m_iconActive : m_iconNormal);

    if (checked)
        emit onPageChanged(layout()->indexOf(pageButton));
}
