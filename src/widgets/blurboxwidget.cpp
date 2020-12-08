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
    , m_blurGroup(new  DBlurEffectGroup)
    , m_blurBackground(new DBlurEffectWidget(this))
    , m_bg(new MaskQWidget(this))
{
    setLayout(m_vLayout);

    m_blurBackground->setAccessibleName(QString("%1 blurBackground").arg(m_name));
    m_bg->setAccessibleName(QString("%1 maskBackground").arg(m_name));
    m_maskLayer->setAccessibleName(QString("%1 maskLayer").arg(m_name));

    m_blurBackground->setFixedSize(m_calcUtil->getAppBoxSize());
    m_blurBackground->setMaskColor(DBlurEffectWidget::LightColor);

    m_blurBackground->setMaskAlpha(DLauncher::APPHBOX_ALPHA);
    m_blurBackground->setBlurRectXRadius(DLauncher::APPHBOX_RADIUS);
    m_blurBackground->setBlurRectYRadius(DLauncher::APPHBOX_RADIUS);
    m_blurBackground->raise();
    m_blurGroup->addWidget(m_blurBackground);

    m_categoryTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_categoryTitle->setFixedHeight(60);

    m_vLayout->setContentsMargins(0, 24, 0, 0);
    m_vLayout->setAlignment(Qt::AlignTop);
    layoutAddWidget(m_categoryTitle, m_calcUtil->getAppBoxSize().width() / 2, Qt::AlignHCenter);
    m_vLayout->addWidget(m_categoryMultiPagesView);

    m_bg->setFixedSize(m_calcUtil->getAppBoxSize());
    m_bg->setColor(new QColor(255,255,255,25));
    m_bg->lower();
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
        mousePos = QCursor::pos();
    }
    QWidget::mousePressEvent(e);
}

void BlurBoxWidget::mouseReleaseEvent(QMouseEvent *e)
{
    QPoint fullscreenPoint = QCursor::pos();
    //单击事件
    if( (e->button() == Qt::LeftButton && fullscreenPoint == mousePos)) {
        emit maskClick(m_category);
        return;
    } else  if ( e->button() == Qt::LeftButton && e->source() == Qt::MouseEventSynthesizedByQt) {
        // 处理触屏点击事件
        int diff_x = qAbs(fullscreenPoint.x() - mousePos.x());
        int diff_y = qAbs(fullscreenPoint.y() - mousePos.y());
        if (diff_x < DLauncher::TOUCH_DIFF_THRESH && diff_y < DLauncher::TOUCH_DIFF_THRESH) {
            emit maskClick(m_category); //处理触屏点击事件
            return;
        }
    }
    //把事件往下传fullscreenframe处理
    QWidget::mouseReleaseEvent(e);
}

void BlurBoxWidget::setMaskSize(QSize size)
{
    m_bg->setFixedSize(size);
    m_maskLayer->setFixedSize(size);
    m_maskLayer->raise();
    m_blurBackground->setFixedSize(size);
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

void BlurBoxWidget::setFixedSize(const QSize &size)
{
    QWidget::setFixedSize(size);
    setMaskSize(size);
    m_categoryTitle->setFixedWidth(size.width());
    m_categoryMultiPagesView->setFixedSize(size - QSize(0,m_categoryTitle->height() + 24));
    m_categoryMultiPagesView->updatePosition();
}

void BlurBoxWidget::setMaskVisible(bool visible)
{
    //设置标题的文本的透明度，icon还需要在deegate中设置
    if (visible) {
       m_categoryTitle->setTitleOpacity(0.3);
    } else {
       m_categoryTitle->setTitleOpacity(1);
    }
    m_maskLayer->setVisible(visible);
}
