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
 * @brief BoxFrame::scaledBackground
 */
void BoxFrame::scaledBlurBackground()
{
    if (m_useSolidBackground)
        return;
        
    QPixmap pixmap(m_lastBlurUrl);
    if (pixmap.isNull())
        pixmap.load(m_defaultBg);
    if (pixmap.isNull())
        return;

    const QSize &size = currentScreen()->size() * currentScreen()->devicePixelRatio();
    QPixmap scaledpixmap = pixmap.scaled(size,
                               Qt::KeepAspectRatioByExpanding,
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

    QPixmap pixmap(m_lastUrl);
    if (pixmap.isNull())
        pixmap = QPixmap(m_defaultBg);
    if (pixmap.isNull())
        return;

    const QSize &size = currentScreen()->size() * currentScreen()->devicePixelRatio();
    m_pixmap = pixmap.scaled(size,
                                Qt::KeepAspectRatioByExpanding,
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
