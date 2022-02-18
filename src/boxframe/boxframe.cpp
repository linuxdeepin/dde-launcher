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
#include "util.h"
#include "constants.h"

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
    , m_bgManager(nullptr)
    , m_useSolidBackground(false)
{
    m_useSolidBackground = getDConfigValue("use-solid-background", false).toBool();
    if (m_useSolidBackground)
        return;

    m_bgManager = new BackgroundManager(this);
    connect(m_bgManager, &BackgroundManager::currentWorkspaceBackgroundChanged, this, &BoxFrame::setBackground);
    connect(m_bgManager, &BackgroundManager::currentWorkspaceBlurBackgroundChanged, this, &BoxFrame::setBlurBackground);
}

void BoxFrame::setBackground(const QString &url)
{
    if (m_lastUrl == url)
        return;

    m_lastUrl = url;

    scaledBackground();
}

void BoxFrame::setBlurBackground(const QString &url)
{
    if (m_lastBlurUrl == url)
        return;

    m_lastBlurUrl = url;

    scaledBlurBackground();
}

/** 缩放图片并缓存
 * @brief BoxFrame::scaledBlurBackground
 */
void BoxFrame::scaledBlurBackground()
{
    if (m_useSolidBackground)
        return;

    const QSize &size = currentScreen()->size() * currentScreen()->devicePixelRatio();
    static QString blurBgPath;
    static QSize lastSize;

    // 当背景图片路径且屏幕大小没有变化，则无需再次加载,减少资源加载耗时
    if (blurBgPath == m_lastBlurUrl && size == lastSize)
        return;

    blurBgPath = m_lastBlurUrl;
    lastSize = size;

    QPixmap pixmap(m_lastBlurUrl);
    if (pixmap.isNull())
        pixmap.load(m_defaultBg);

    QPixmap scaledpixmap = pixmap.scaled(size, Qt::KeepAspectRatioByExpanding,
                                         Qt::SmoothTransformation);
    emit backgroundImageChanged(scaledpixmap);
}

/** 缩放图片并缓存
 * @brief BoxFrame::scaledBackground
 */
void BoxFrame::scaledBackground()
{
    if (m_useSolidBackground)
        return;

    // 当背景图片路径且屏幕大小没有变化，则无需再次加载,减少资源加载耗时
    const QSize &size = currentScreen()->size() * currentScreen()->devicePixelRatio();
    static QString bgPath;
    static QSize lastSize;

    if (bgPath == m_lastUrl && lastSize == size)
        return;

    bgPath = m_lastUrl;
    lastSize = size;

    QPixmap pixmap(m_lastUrl);
    if (pixmap.isNull())
        pixmap = QPixmap(m_defaultBg);

    m_pixmap = pixmap.scaled(size, Qt::KeepAspectRatioByExpanding,
                             Qt::SmoothTransformation);
    update();
}

const QScreen *BoxFrame::currentScreen()
{
    if (DisplayHelper::instance()->displayMode() == MERGE_MODE)
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
    if (!m_useSolidBackground && !m_pixmap.isNull()) {
        painter.drawPixmap(screenSize, m_pixmap, QRect(screenSize.topLeft(),
                                                    screenSize.size() * currentScreen()->devicePixelRatio()));
    } else {
        painter.fillRect(screenSize, QColor(DLauncher::SOLID_BACKGROUND_COLOR));
    }
}

void BoxFrame::moveEvent(QMoveEvent *event)
{
    if (m_bgManager)
        m_bgManager->updateBlurBackgrounds();
    QLabel::moveEvent(event);
}
