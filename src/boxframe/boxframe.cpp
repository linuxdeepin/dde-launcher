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

#include <QDebug>
#include <QPushButton>
#include <QScreen>
#include <QTimer>
#include <QCryptographicHash>

#include "boxframe.h"

static const QString DefaultBackground = "/usr/share/backgrounds/default_background.jpg";

BoxFrame::BoxFrame(QWidget *parent)
    : QFrame(parent)
{
}

BoxFrame::BoxFrame(const QString &url, QWidget *parent)
    : BoxFrame(parent)
{
    this->setBackground(url);
}

BoxFrame::~BoxFrame()
{
}

// Message for maintainers: DON'T use QSS to set the background !

// This function takes ~2ms by setting QSS to set backgrounds, but causes show() of
// ShutdownFrame takes ~260ms to complete. On the other hand, this function takes
// ~130ms by setting pixmap, yet takes only ~12ms to complete the show() of ShutdownFrame.
// It'll be more obvious on dual screens environment.
void BoxFrame::setBackground(const QString &url, bool force)
{
    if (m_lastUrl == url && !force) return;

    m_lastUrl = url;
    QString path = QUrl(url).isLocalFile() ? QUrl(url).toLocalFile() : url;

    if (!QFile::exists(path)) {
        path = DefaultBackground;
    }

    QPixmap pix(path);

    if (pix.isNull()) {
        pix.load(DefaultBackground);
    }

    m_pixmap = pix;
    m_cache = QPixmap();
    emit backgroundChanged();
}

QPixmap BoxFrame:: backgroundPixmap()
{
    if (m_cache.isNull() || size() != m_cache.size()) {
        QPixmap cache = m_pixmap.scaled(size(), Qt::KeepAspectRatioByExpanding);

        QRect copyRect((cache.width() - size().width()) / 2,
                       (cache.height() - size().height()) / 2,
                       size().width(), size().height());

        m_cache = cache.copy(copyRect);
    }

    return m_cache;
}
