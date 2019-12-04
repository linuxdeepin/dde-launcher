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

    addWidget(this);
}

void AppListArea::addWidget(QWidget * view) {
    view->installEventFilter(this);

    m_views << view;
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
        // 默认是上下滚动，这里改成左右滚动
        //QWheelEvent *event = new QWheelEvent(e->pos(), e->delta(), e->buttons(), e->modifiers(),Qt::Horizontal);
        QScrollArea::wheelEvent(e);
    }
}

void AppListArea::enterEvent(QEvent *e)
{
    QScrollArea::enterEvent(e);

    emit mouseEntered();
}

bool AppListArea::eventFilter(QObject *watched, QEvent *e)
{
    QWidget *    view  = qobject_cast<QWidget *>(watched);
    QMouseEvent *event = static_cast<QMouseEvent *>(e);

    if (view && m_views.contains(view) && event) {
        if (event->source() == Qt::MouseEventSynthesizedByQt) {
            switch (event->type()) {
                case QMouseEvent::MouseButtonPress:
                    m_lastTouchBeginPos = event->pos();

                    if (m_updateEnableSelectionByMouseTimer) {
                        m_updateEnableSelectionByMouseTimer->stop();
                    }
                    else {
                        m_updateEnableSelectionByMouseTimer = new QTimer(this);
                        m_updateEnableSelectionByMouseTimer->setSingleShot(true);

                        static QObject *theme_settings =
                            reinterpret_cast<QObject *>(qvariant_cast<quintptr>(qApp->property("_d_theme_settings_object")));
                        QVariant touchFlickBeginMoveDelay;

                        if (theme_settings) {
                            touchFlickBeginMoveDelay = theme_settings->property("touchFlickBeginMoveDelay");
                        }

                        m_updateEnableSelectionByMouseTimer->setInterval(touchFlickBeginMoveDelay.isValid() ? touchFlickBeginMoveDelay.toInt()
                                                                                                             : 300);

                        connect(m_updateEnableSelectionByMouseTimer, &QTimer::timeout, this, [=] {
                            m_updateEnableSelectionByMouseTimer->deleteLater();
                            m_updateEnableSelectionByMouseTimer = nullptr;
                        });
                    }
                    m_updateEnableSelectionByMouseTimer->start();
                    break;
                case QMouseEvent::MouseButtonRelease:
                    if (QScroller::hasScroller(this)) {
                        return true;
                    }
                    break;
                case QMouseEvent::MouseMove:
                    if (QScroller::hasScroller(this)) {
                        return true;
                    }

                    if (m_updateEnableSelectionByMouseTimer && m_updateEnableSelectionByMouseTimer->isActive()) {
                        const QPoint difference_pos = event->pos() - m_lastTouchBeginPos;
                        if (qAbs(difference_pos.x()) < touchTapDistance && qAbs(difference_pos.y()) > touchTapDistance) {
                            QScroller::grabGesture(this);
                            QScroller *scroller = QScroller::scroller(this);

                            const QPointF point = view->mapTo(this, event->localPos().toPoint());

                            scroller->handleInput(QScroller::InputPress, point, event->timestamp());
                            scroller->handleInput(QScroller::InputMove, point, event->timestamp());

                            connect(scroller, &QScroller::stateChanged, this, [=](QScroller::State newstate) {
                                if (newstate == QScroller::Inactive) {
                                    QScroller::scroller(this)->deleteLater();
                                }
                            });
                            return true;
                        }
                        return false;
                    }
                    break;
                default: break;
            }
        }
    }
    return QScrollArea::eventFilter(watched, e);
}
