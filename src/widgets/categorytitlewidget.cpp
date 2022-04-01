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

#include "categorytitlewidget.h"
#include "constants.h"
#include "util.h"
#include <DFontSizeManager>
#include <QHBoxLayout>
#include <QGuiApplication>

DWIDGET_USE_NAMESPACE
CategoryTitleWidget::CategoryTitleWidget(const QString &title, QWidget *parent) :
    QFrame(parent),
    m_calcUtil(CalculateUtil::instance()),
    m_title(new QLabel(this)),
    m_opacityAnimation(new QPropertyAnimation(this, "titleOpacity", this)),
    m_titleOpacity(0)
{
    m_title->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_title->setAccessibleName(title);
    setText(title);
    DFontSizeManager::instance()->bind(m_title, DFontSizeManager::T1);
    setTitleOpacity(1);  // update the style of this widget by force.
    m_opacityAnimation->setDuration(300);

    setAccessibleName(title);
    setFixedHeight(DLauncher::CATEGORY_TITLE_WIDGET_HEIGHT);

    //addTextShadow();
    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, &CategoryTitleWidget::relayout);

    connect(qApp, &QGuiApplication::fontChanged, this, &CategoryTitleWidget::relayout);
}

void CategoryTitleWidget::setText(const QString &title)
{
    QFontMetrics fontMetric(m_title->font());
    const int width = fontMetric.width(title);
    m_title->setFixedWidth(width + 10);
    m_title->setText(title);
}

void CategoryTitleWidget::updatePosition(const QPoint pos, int w, int posType)
{
    int x = width() / 2 - m_title->width() / 2;

    int boxWidth = m_calcUtil->getAppBoxSize().width();
    int canMoveWidth = boxWidth / 2 - m_title->width() / 2 - DLauncher::APPS_CATEGORY_TITLE_SPACING;

    int leftX  = w / 2 - boxWidth / 2 - DLauncher::APPHBOX_SPACING;
    int rightX = w / 2 + boxWidth / 2 + DLauncher::APPHBOX_SPACING;

    //因为单元循环引用冲突，没有直接引用scrollwidgetagent.h 单元中的枚举PosType
    //其中0 = Pos_None,1 = Pos_LL, 2 = Pos_L,3 = Pos_M,4 = Pos_R,5 = Pos_RR
    if (posType == 1) {
        x = width() - m_title->width() - DLauncher::APPS_CATEGORY_TITLE_SPACING;
    } else if (posType == 2) {
        x = width() - m_title->width() - DLauncher::APPS_CATEGORY_TITLE_SPACING;

        int movedDiff = pos.x() + boxWidth - leftX;

        movedDiff = movedDiff < 0 ? 0 : movedDiff;
        movedDiff = movedDiff > canMoveWidth ? canMoveWidth : movedDiff;

        x = x - movedDiff;
    } else if (posType == 4) {
        x = DLauncher::APPS_CATEGORY_TITLE_SPACING;

        int movedDiff = pos.x() - rightX;
        movedDiff = movedDiff > 0 ? 0 : movedDiff;
        movedDiff = movedDiff < -canMoveWidth ? -canMoveWidth : movedDiff;

        x = x - movedDiff;
    } else if (posType == 5) {
        x = DLauncher::APPS_CATEGORY_TITLE_SPACING;
    }

    QPoint p(x, 0);
    m_title->move(p);
    update();
}

void CategoryTitleWidget::relayout()
{
    QFont titleFont(m_title->font());
    QFontMetrics fontMetric(titleFont);
    const int width = fontMetric.width(m_title->text());
    m_title->setFixedWidth(width + 10);
}

qreal CategoryTitleWidget::titleOpacity() const
{
    return m_titleOpacity;
}

void CategoryTitleWidget::setTitleOpacity(const qreal &titleOpacity)
{
    if (m_titleOpacity != titleOpacity) {
        m_titleOpacity = titleOpacity;

        QPalette p = m_title->palette();
        p.setColor(m_title->foregroundRole(), QColor::fromRgbF(1, 1, 1, m_titleOpacity));
        p.setColor(m_title->backgroundRole(), Qt::transparent);
        m_title->setPalette(p);
    }
}

QLabel *CategoryTitleWidget::textLabel()
{
    return m_title;
}
