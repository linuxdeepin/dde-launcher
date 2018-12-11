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

#include "applistarea.h"

#include <QDebug>
#include <QWheelEvent>
#include <QScroller>
#include <QTimer>
#include <private/qguiapplication_p.h>
#include <qpa/qplatformtheme.h>

AppListArea::AppListArea(QWidget *parent)
    : QScrollArea(parent)
    , m_updateEnableSelectionByMouseTimer(nullptr)
{
#if QT_VERSION < QT_VERSION_CHECK(5,9,0)
    touchTapDistance = 15;
#else
    touchTapDistance = QGuiApplicationPrivate::platformTheme()->themeHint(QPlatformTheme::TouchDoubleTapDistance).toInt();
#endif
}

void AppListArea::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers().testFlag(Qt::ControlModifier))
    {
        e->accept();

        if (e->delta() > 0)
            emit increaseIcon();
        else
            emit decreaseIcon();
    } else {
        QScrollArea::wheelEvent(e);
    }
}

void AppListArea::enterEvent(QEvent *e)
{
    QScrollArea::enterEvent(e);

    emit mouseEntered();
}

// void AppListArea::mousePressEvent(QMouseEvent *event) {
//     // 当source为MouseEventSynthesizedByQt时，认为正在使用触屏，开始手动控制触摸操作
//     if (event->source() == Qt::MouseEventSynthesizedByQt) {
//         m_lastTouchBeginPos = event->pos();

//         if (m_updateEnableSelectionByMouseTimer) {
//             m_updateEnableSelectionByMouseTimer->stop();
//         }
//         else {
//             m_updateEnableSelectionByMouseTimer = new QTimer(this);
//             m_updateEnableSelectionByMouseTimer->setSingleShot(true);
//             m_updateEnableSelectionByMouseTimer->setInterval(300);

//             connect(m_updateEnableSelectionByMouseTimer, &QTimer::timeout, this, [=] {
//                 m_updateEnableSelectionByMouseTimer->deleteLater();
//                 m_updateEnableSelectionByMouseTimer = nullptr;
//             });
//         }
//         m_updateEnableSelectionByMouseTimer->start();
//     }

//     QScrollArea::mousePressEvent(event);
// }

// void AppListArea::mouseReleaseEvent(QMouseEvent *event) {
//     if (!QScroller::hasScroller(this)) {
//         QScrollArea::mouseReleaseEvent(event);
//     }
// }

// void AppListArea::mouseMoveEvent(QMouseEvent *event) {
//     if (event->source() == Qt::MouseEventSynthesizedByQt) {
//         if (QScroller::hasScroller(this)) {
//             return;
//         }

//         if (m_updateEnableSelectionByMouseTimer && m_updateEnableSelectionByMouseTimer->isActive()) {
//             const QPoint difference_pos = event->pos() - m_lastTouchBeginPos;
//             if (qAbs(difference_pos.x()) > touchTapDistance || qAbs(difference_pos.y()) > touchTapDistance) {
//                 QScroller::grabGesture(this);
//                 QScroller *scroller = QScroller::scroller(this);

//                 connect(scroller, &QScroller::stateChanged, this, [=] (QScroller::State newstate) {
//                     if (newstate == QScroller::Inactive)  {
//                         QScroller::scroller(this)->deleteLater();
//                     }
//                 });
//                 scroller->handleInput(QScroller::InputPress, event->localPos(), event->timestamp());
//                 scroller->handleInput(QScroller::InputMove, event->localPos(), event->timestamp());
//             }
//             return;
//         }
//     }

//     return QScrollArea::mouseMoveEvent(event);
// }
