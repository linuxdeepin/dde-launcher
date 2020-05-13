/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     niecheng <niejiashan@163.com>
 *
 * Maintainer: niecheng <niejiashan@163.com>
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

#include "blurboxwidget.h"
#include "src/global_util/calculate_util.h"

#include <QPainter>

DWIDGET_USE_NAMESPACE
BlurBoxWidget::BlurBoxWidget(AppsListModel::AppCategory curCategory, char *name, QWidget *parent)
    : QWidget (parent)
    , m_vLayout(new QVBoxLayout(this))
    , m_maskLayer(new MaskQWidget(this))
    , m_calcUtil(CalculateUtil::instance())
    , m_category(curCategory)
    , m_name(name)
    , m_categoryMultiPagesView(new MultiPagesView(curCategory))
    , m_categoryTitle(new CategoryTitleWidget(QApplication::translate("MiniCategoryWidget", name)))
    ,m_titleOpacityEffect(new QGraphicsOpacityEffect)
    ,m_pagesOpacityEffect(new QGraphicsOpacityEffect)
    ,m_blurGroup(new  DBlurEffectGroup)
    ,m_blurBackground(new DBlurEffectWidget(this))
    ,m_bg(new MaskQWidget(this))
{
    setLayout(m_vLayout);

    m_blurBackground->setFixedSize(m_calcUtil->getAppBoxSize());
    m_blurBackground->setMaskColor(DBlurEffectWidget::AutoColor);
    m_blurBackground->setMaskAlpha(DLauncher::APPHBOX_ALPHA);
    m_blurBackground->setBlurRectXRadius(DLauncher::APPHBOX_RADIUS);
    m_blurBackground->setBlurRectYRadius(DLauncher::APPHBOX_RADIUS);
    m_blurGroup->addWidget(m_blurBackground);

    m_vLayout->setContentsMargins(0, 0, 0, 0);
    m_vLayout->setAlignment(Qt::AlignTop);
    layoutAddWidget(m_categoryTitle, m_calcUtil->getAppBoxSize().width() / 2, Qt::AlignHCenter);
    m_vLayout->addWidget(m_categoryMultiPagesView);
    m_categoryMultiPagesView->setGraphicsEffect(m_pagesOpacityEffect);
    m_categoryTitle->setGraphicsEffect(m_titleOpacityEffect);

    m_bg->setFixedSize(m_calcUtil->getAppBoxSize());
    m_bg->setColor(new QColor(255,255,255,25));
}

void BlurBoxWidget::layoutAddWidget(QWidget *child)
{
    m_vLayout->addWidget(child);
}

void BlurBoxWidget::layoutAddWidget(QWidget *child, int stretch, Qt::Alignment alignment)
{
    m_vLayout->addWidget(child, stretch, alignment);
}

MultiPagesView *BlurBoxWidget::getMultiPagesView()
{
    return m_categoryMultiPagesView;
}

void BlurBoxWidget::updateBackBlurPos(QPoint p)
{
    m_blurGroup->addWidget(m_blurBackground,p);
}

void BlurBoxWidget::updateBackgroundImage(const QPixmap &img)
{
    m_blurGroup->setSourceImage(img.toImage(),0);
}

void BlurBoxWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        mousePos = e->pos();
    }
}

void BlurBoxWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && e->pos() == mousePos) {
        AppsListModel::scrollType nType = AppsListModel::FirstShow;

        if (m_calcUtil->getScreenSize().width() * 3 / 4 < e->globalX()) {
            nType = AppsListModel::ScrollRight;
        } else if (m_calcUtil->getScreenSize().width() / 4 > e->globalX()){
            nType = AppsListModel::ScrollLeft;
        } else {
            emit hideLauncher();
            return;
        }
        emit maskClick(m_category, nType);
    }
}

void BlurBoxWidget::setMaskSize(QSize size)
{
    m_bg->setFixedSize(this->size()+QSize(0,8));
	setFixedWidth(size.width());
    m_maskLayer->setFixedSize(size);
    m_maskLayer->raise();
    m_blurBackground->setFixedSize(this->size());
}

void BlurBoxWidget::setDataDelegate(QAbstractItemDelegate *delegate)
{
    m_categoryMultiPagesView->setAccessibleName(m_name);
    m_categoryMultiPagesView->setDataDelegate(delegate);
    m_categoryMultiPagesView->updatePageCount(m_category);
}

void BlurBoxWidget::setBlurBgVisible(bool visible)
{
    m_blurBackground->setVisible(visible);
}

void BlurBoxWidget::setMaskVisible(bool visible)
{
    m_maskLayer->setVisible(visible);
    double opacity = 0.3;
    visible? opacity= 0.3:opacity= 0.99;
    m_titleOpacityEffect->setOpacity(opacity);
    m_pagesOpacityEffect->setOpacity(opacity);
}


