/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
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

#include "backgroundmanager.h"

#include <unistd.h>

using namespace com::deepin;

static const QString DefaultWallpaper = "/usr/share/backgrounds/default_background.jpg";

static QString getLocalFile(const QString &file) {
    const QUrl url(file);
    return url.isLocalFile() ? url.toLocalFile() : url.url();
}

BackgroundManager::BackgroundManager(QObject *parent)
    : QObject(parent)
    , m_currentWorkspace(-1)
    , m_wmInter(new wm("com.deepin.wm", "/com/deepin/wm", QDBusConnection::sessionBus(), this))
    , m_blurInter(new ImageBlurInter("com.deepin.daemon.Accounts",
                                     "/com/deepin/daemon/ImageBlur",
                                     QDBusConnection::systemBus(), this))
    , m_currentUser(new User("com.deepin.daemon.Accounts",
                             QString("/com/deepin/daemon/Accounts/User%1").arg(getuid()),
                             QDBusConnection::systemBus(), this))
{
    m_blurInter->setSync(false, false);
    m_currentUser->setSync(false, false);

    auto updateWorkspace = [this] (int, int to) {
        setCurrentWorkspace(to);
    };

    connect(m_wmInter, &__wm::WorkspaceSwitched, updateWorkspace);
    connect(m_blurInter, &ImageBlurInter::BlurDone, this, &BackgroundManager::onBlurDone);
    connect(m_currentUser, &User::DesktopBackgroundsChanged, this, &BackgroundManager::onDesktopWallpapersChanged);

    onDesktopWallpapersChanged(m_currentUser->desktopBackgrounds());
}

QString BackgroundManager::currentWorkspaceBackground() const
{
    if (m_backgrounds.isEmpty()) return DefaultWallpaper;

    const QString &source = m_backgrounds[m_currentWorkspace];
    const QString &path = QUrl(source).isLocalFile() ? QUrl(source).toLocalFile() : source;
    const QString &s = m_blurInter->Get(QFile::exists(path) ? path : DefaultWallpaper);

    return s.isEmpty() ? DefaultWallpaper : s;
}

void BackgroundManager::onBlurDone(const QString &source, const QString &blur, bool done)
{
    if (m_currentWorkspace == -1) {
        emit currentWorkspaceBackgroundChanged(blur);
    }
    else {
        const QString &current = m_backgrounds[m_currentWorkspace];

        const QString &currentPath = QUrl(current).isLocalFile() ? QUrl(current).toLocalFile() : current;
        const QString &sourcePath = QUrl(source).isLocalFile() ? QUrl(source).toLocalFile() : source;

        if (done && QFile::exists(blur) && currentPath == sourcePath)
            emit currentWorkspaceBackgroundChanged(blur);
    }
}

int BackgroundManager::currentWorkspace() const
{
    return m_currentWorkspace;
}

void BackgroundManager::setCurrentWorkspace(int currentWorkspace)
{
    if (m_currentWorkspace != currentWorkspace) {
        m_currentWorkspace = currentWorkspace;
        updateBackgrounds();
    }
}

void BackgroundManager::updateBackgrounds()
{
    QString path;
    if (m_currentWorkspace == -1) {
        path = getLocalFile(m_wmInter->GetCurrentWorkspaceBackground());
    }
    else {
        const QString &current = m_backgrounds[m_currentWorkspace];
        path = getLocalFile(current);
    }

    path = QFile::exists(path) ? path : DefaultWallpaper;

    const QString &file = m_blurInter->Get(path);

    emit currentWorkspaceBackgroundChanged(file.isEmpty() ? path : file);
}

void BackgroundManager::onDesktopWallpapersChanged(const QStringList &files)
{
    m_backgrounds.clear();

    for (const QString &path : files) {
        m_backgrounds << getLocalFile(path);
    }

    updateBackgrounds();
}
