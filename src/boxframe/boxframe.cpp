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

/**
 * @brief BoxFrame::BoxFrame 桌面背景类
 * @param parent 父对象
 */
BoxFrame::BoxFrame(QWidget *parent)
    : QLabel(parent)
    , m_defaultBg("/usr/share/backgrounds/default_background.jpg")
    , m_bgManager(new BackgroundManager(this))
{
    QPixmapCache::setCacheLimit(10240000);
    connect(m_bgManager, &BackgroundManager::currentWorkspaceBackgroundChanged, this, &BoxFrame::setBackground);
    connect(m_bgManager, &BackgroundManager::currentWorkspaceBlurBackgroundChanged, this, &BoxFrame::setBlurBackground);
}

// Message for maintainers: DON'T use QSS to set the background !

// This function takes ~2ms by setting QSS to set backgrounds, but causes show() of
// ShutdownFrame takes ~260ms to complete. On the other hand, this function takes
// ~130ms by setting pixmap, yet takes only ~12ms to complete the show() of ShutdownFrame.
// It'll be more obvious on dual screens environment.
void BoxFrame::setBackground(const QString &url)
{
    if (m_lastUrl == url)
        return;

    m_lastUrl = url;

    // 背景发生变化时需要重新生成背景样式
    QPixmapCache::remove(m_cacheNormalKey);

    m_pixmap = QPixmap(url);
    if (m_pixmap.isNull())
        m_pixmap.load(m_defaultBg);

    updateBackground();
}

void BoxFrame::setBlurBackground(const QString &url)
{
    if (m_lastBlurUrl == url)
        return;

    m_lastBlurUrl = url;

    // 背景发生变化时需要重新生成模糊背景样式
    QPixmapCache::remove(m_cacheBlurKey);

    QPixmap pix(url);
    if (pix.isNull())
        pix.load(m_defaultBg);

    const QSize &size = currentScreen()->size() * currentScreen()->devicePixelRatio();

    QPixmap blurCache;
    if (!QPixmapCache::find(m_cacheBlurKey, &blurCache)) {
        blurCache = pix.scaled(size,
                               Qt::KeepAspectRatioByExpanding,
                               Qt::SmoothTransformation);

        m_cacheBlurKey = QPixmapCache::insert(blurCache);
    }

    emit backgroundImageChanged(blurCache);
}

/**当桌面发生旋转、背景更换时需要更新缓存
 * @brief BoxFrame::removeCache
 */
void BoxFrame::removeCache()
{
    QPixmapCache::remove(m_cacheNormalKey);
    QPixmapCache::remove(m_cacheBlurKey);
}

/**更新桌面模糊背景,桌面背景不改变时进行缓存桌面模糊背景
 * @brief BoxFrame::updateBlurBackground
 */
void BoxFrame::updateBlurBackground()
{
    QPixmap pix(m_lastBlurUrl);

    if (pix.isNull())
        pix.load(m_defaultBg);

    const QSize &size = currentScreen()->size() * currentScreen()->devicePixelRatio();

    QPixmap cache = pix.scaled(size,
                               Qt::KeepAspectRatioByExpanding,
                               Qt::SmoothTransformation);

    emit backgroundImageChanged(cache);
}

/** 更新桌面背景,桌面背景不改变时进行缓存桌面背景
 * @brief BoxFrame::updateBackground
 */
void BoxFrame::updateBackground()
{
    const QSize &size = currentScreen()->size() * currentScreen()->devicePixelRatio();

    if (m_pixmap.isNull()) {
        m_pixmap.load(m_defaultBg);
        QPixmapCache::remove(m_cacheNormalKey);
    }

    QPixmap cache;
    if (!QPixmapCache::find(m_cacheNormalKey, &cache)) {
        cache = m_pixmap.scaled(size,
                                Qt::KeepAspectRatioByExpanding,
                                Qt::SmoothTransformation);
        m_cacheNormalKey = QPixmapCache::insert(cache);
    }

    m_cache = cache;
    update();
}

const QScreen *BoxFrame::currentScreen()
{
    if (m_bgManager->dispalyMode() == MERGE_MODE)
        return qApp->primaryScreen();

    int screenIndex = QApplication::desktop()->screenNumber(this);
    QList<QScreen *> screens = qApp->screens();
    if (screenIndex < screens.count())
        return screens[screenIndex];

    return qApp->primaryScreen();
}

void BoxFrame::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    QRect screenSize(QPoint(0, 0), currentScreen()->geometry().size());
    painter.drawPixmap(screenSize, m_cache, QRect(screenSize.topLeft(),
                                                  screenSize.size() * currentScreen()->devicePixelRatio()));
}

void BoxFrame::moveEvent(QMoveEvent *event)
{
    m_bgManager->updateBlurBackgrounds();
    QLabel::moveEvent(event);
}
