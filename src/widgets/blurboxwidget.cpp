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
#include "calculate_util.h"
#include "constants.h"
#include "util.h"

#include <QPainter>
#include <QResizeEvent>

DWIDGET_USE_NAMESPACE
using namespace DLauncher;

QSharedPointer<DBlurEffectGroup> BlurBoxWidget::m_blurGroup = nullptr;

/**
 * @brief BlurBoxWidget::BlurBoxWidget 单个分类应用的内容控件（标题+MultiPagesView）
 * @param curCategory 分类类别枚举值
 * @param name 设置的对象名
 * @param parent 父对象
 */
BlurBoxWidget::BlurBoxWidget(AppsListModel::AppCategory curCategory, char *name, QWidget *parent)
    : QWidget (parent)
    , m_vLayout(new QVBoxLayout(this))
    , m_maskLayer(new MaskQWidget(this))
    , m_calcUtil(CalculateUtil::instance())
    , m_category(curCategory)
    , m_name(name)
    , m_categoryMultiPagesView(new MultiPagesView(curCategory))
    , m_categoryTitle(new CategoryTitleWidget(QApplication::translate("MiniCategoryWidget", name)))
    , m_blurBackground(new DBlurEffectWidget(this))
    , m_bg(new MaskQWidget(this))
{
    if (m_blurGroup.isNull())
        m_blurGroup = QSharedPointer<DBlurEffectGroup>(new DBlurEffectGroup);

    setLayout(m_vLayout);

    m_blurBackground->setAccessibleName(QString("%1 blurBackground").arg(m_name));
    m_bg->setAccessibleName(QString("%1 maskBackground").arg(m_name));
    m_maskLayer->setAccessibleName(QString("%1 maskLayer").arg(m_name));

    m_blurBackground->setFixedSize(m_calcUtil->getAppBoxSize());
    m_blurBackground->setMaskColor(DBlurEffectWidget::LightColor);

    int maskAlpha = getDConfigValue("use-solid-background", false).toBool() ? 0 : DLauncher::APPHBOX_ALPHA;
    m_blurBackground->setMaskAlpha(maskAlpha);
    m_blurBackground->setBlurRectXRadius(DLauncher::APPHBOX_RADIUS);
    m_blurBackground->setBlurRectYRadius(DLauncher::APPHBOX_RADIUS);
    m_blurBackground->raise();
    m_blurGroup->addWidget(m_blurBackground);

    m_categoryTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_categoryTitle->setFixedHeight(70);

    // 应用分类标题
    layoutAddWidget(m_categoryTitle, m_calcUtil->getAppBoxSize().width() / 2, Qt::AlignHCenter);

    // 单个应用分类控件
    m_vLayout->addWidget(m_categoryMultiPagesView);

    m_bg->setFixedSize(m_calcUtil->getAppBoxSize());
    m_bg->setColor(QColor(255, 255, 255, 25));
    m_bg->lower();

//    setStyleSheet("QWidget{background: red;}");
    m_categoryMultiPagesView->installEventFilter(this);
}

BlurBoxWidget::~BlurBoxWidget()
{
    delete m_categoryMultiPagesView;
    m_categoryMultiPagesView = nullptr;
}

void BlurBoxWidget::layoutAddWidget(QWidget *child, int stretch, Qt::Alignment alignment)
{
    m_vLayout->addWidget(child, stretch, alignment);
}

/** 获取单个应用类别控件下的视图控件
 * @brief BlurBoxWidget::getMultiPagesView
 * @return
 */
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

void BlurBoxWidget::resizeEvent(QResizeEvent *e)
{
    QSize boxwidgetSize = m_categoryTitle->size() + m_categoryMultiPagesView->size() + QSize(0, 70);
    setFixedSize(m_categoryTitle->size() + m_categoryMultiPagesView->size() + QSize(0, 70));

    qInfo() << __FUNCTION__ << __LINE__ << boxwidgetSize;
    QWidget::resizeEvent(e);
}

bool BlurBoxWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_categoryMultiPagesView) {
//        QResizeEvent *event = qobject_cast<QResizeEvent *>(event);
        if (event->type() == QEvent::Resize) {
//            QResizeEvent *sizeEvent = static_cast<QResizeEvent *>(event);
            QSize boxwidgetSize = m_categoryTitle->size() + m_categoryMultiPagesView->size() + QSize(0, 70);
            qInfo() << "boxwidgetSize:" << boxwidgetSize;
            setFixedSize(boxwidgetSize);
            return false;
        }
    }
    return QWidget::eventFilter(watched, event);
}

//QSize BlurBoxWidget::sizeHint() const
//{
//    QSize boxwidgetSize = m_categoryTitle->size() + m_categoryMultiPagesView->size() + QSize(0, 70);
////    setFixedSize(m_categoryTitle->size() + m_categoryMultiPagesView->size() + QSize(0, 70));

////    qInfo() <<__FUNCTION__ << __LINE__ << ",boxwidgetsize:" << boxwidgetSize;
//    return boxwidgetSize;
//    //    QWidget::sizeHint();
//}

//void BlurBoxWidget::changeEvent(QEvent *event)
//{
////    event->type() ==
//}

void BlurBoxWidget::setMaskSize(QSize size)
{
    m_bg->setFixedSize(size);
    m_maskLayer->setFixedSize(size);
    m_maskLayer->raise();
    m_blurBackground->setFixedSize(size);
}

/**
 * @brief BlurBoxWidget::setDataDelegate 应用分类下视图设置代理
 * @param delegate 视图代理
 */
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
