/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
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

#include "boxframe.h"
#include "backgroundmanager.h"

#include <QDebug>
#include <QUrl>
#include <QFile>
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QPaintEvent>

static const QString DefaultBackground = "/usr/share/backgrounds/default_background.jpg";

BoxFrame::BoxFrame(QWidget *parent)
    : QLabel(parent)
    , m_bgManager(new BackgroundManager(this))
    , m_displayInter(new DBusDisplay(this))
{
    connect(m_bgManager, &BackgroundManager::currentWorkspaceBackgroundChanged, this, &BoxFrame::setBackground);
}

BoxFrame::~BoxFrame()
{
}

// Message for maintainers: DON'T use QSS to set the background !

// This function takes ~2ms by setting QSS to set backgrounds, but causes show() of
// ShutdownFrame takes ~260ms to complete. On the other hand, this function takes
// ~130ms by setting pixmap, yet takes only ~12ms to complete the show() of ShutdownFrame.
// It'll be more obvious on dual screens environment.
void BoxFrame::setBackground(const QString &url)
{
    if (m_lastUrl == url) return;

    m_lastUrl = url;

    QPixmap pix(url);

    if (pix.isNull()) {
        pix.load(DefaultBackground);
    }

    m_pixmap = pix;

    updateBackground();
}

const QPixmap BoxFrame::backgroundPixmap() {
    const QSize &size = QSize(m_displayInter->primaryRect().width, m_displayInter->primaryRect().height) * qApp->primaryScreen()->devicePixelRatio();
    QPixmap cache = m_pixmap.scaled(size,
                                    Qt::KeepAspectRatioByExpanding,
                                    Qt::SmoothTransformation);

    QRect copyRect((cache.width() - size.width()) / 2,
                   (cache.height() - size.height()) / 2,
                   size.width(), size.height());

    cache = cache.copy(copyRect);
    cache.setDevicePixelRatio(devicePixelRatioF());

    return cache;
}

void BoxFrame::updateBackground()
{
    m_cache = backgroundPixmap();

    update();
}

void BoxFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    const QRect &geom = m_displayInter->primaryRect();
    QRect tr(QPoint(0, 0), geom.size());

    painter.drawPixmap(tr,
                       m_cache,
                       QRect(tr.topLeft(),
                             tr.size() * m_cache.devicePixelRatioF()));

}
