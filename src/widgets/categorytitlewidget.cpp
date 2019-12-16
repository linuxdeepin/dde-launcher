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
#include "src/global_util/constants.h"
#include "src/global_util/util.h"
#include <DFontSizeManager>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>

DWIDGET_USE_NAMESPACE
CategoryTitleWidget::CategoryTitleWidget(const QString &title, QWidget *parent) :
    QFrame(parent),
    m_calcUtil(CalculateUtil::instance()),
    m_title(new QLabel(this)),
    m_opacityAnimation(new QPropertyAnimation(this, "titleOpacity"))
{
    QLabel *whiteLine = new QLabel(this);
    whiteLine->setObjectName("CategoryWhiteLine");
    whiteLine->setFixedHeight(1);

    QVBoxLayout *lineLayout = new QVBoxLayout;
    lineLayout->setMargin(0);
    lineLayout->setSpacing(0);
    lineLayout->addStretch();
    lineLayout->addWidget(whiteLine);
    lineLayout->addStretch();

    setText(title);
    DFontSizeManager::instance()->bind(m_title, DFontSizeManager::T1);
    setTitleOpacity(1);  // update the style of this widget by force.
    m_opacityAnimation->setDuration(300);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addSpacing(DLauncher::APPS_CATEGORY_TITLE_SPACING);
    mainLayout->addWidget(m_title);
    mainLayout->addSpacing(DLauncher::APPS_CATEGORY_TITLE_SPACING);

    setAccessibleName(title);
    setLayout(mainLayout);
    setFixedHeight(DLauncher::CATEGORY_TITLE_WIDGET_HEIGHT);

    addTextShadow();

    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, &CategoryTitleWidget::relayout);
}

void CategoryTitleWidget::setTextVisible(const bool visible, const bool animation)
{
    // FIXME: setTextVisible中设置的调色板对应颜色透明后，再次设置为白色却显示不出来
    if (visible) {
        setText(m_text);
    } else {
        m_title->clear();
    }
    return;

    m_opacityAnimation->stop();

    if (!animation) {
        setTitleOpacity(visible ? 1 : 0);
    } else {
        if (visible) {
            m_opacityAnimation->setStartValue(titleOpacity());
            m_opacityAnimation->setEndValue(1);
        } else {
            m_opacityAnimation->setStartValue(titleOpacity());
            m_opacityAnimation->setEndValue(0);
        }
        m_opacityAnimation->start();
    }
}

void CategoryTitleWidget::setText(const QString &title)
{
    m_text = title;

    QFont titleFont(m_title->font());
    titleFont.setPixelSize(m_calcUtil->titleTextSize());
    m_title->setFont(titleFont);

    QFontMetrics fontMetric(m_title->font());
    const int width = fontMetric.width(title);
    m_title->setFixedWidth(width + 10);
    m_title->setText(title);
}

void CategoryTitleWidget::addTextShadow()
{
    QGraphicsDropShadowEffect *textDropShadow = new QGraphicsDropShadowEffect;
    textDropShadow->setBlurRadius(4);
    textDropShadow->setColor(QColor(0, 0, 0, 128));
    textDropShadow->setOffset(0, 2);
    m_title->setGraphicsEffect(textDropShadow);
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
