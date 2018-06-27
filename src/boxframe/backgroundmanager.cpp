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

#include <QGSettings>

using namespace com::deepin;

static const QString DefaultWallpaper = "/usr/share/backgrounds/default_background.jpg";

BackgroundManager::BackgroundManager(QObject *parent)
    : QObject(parent)
    , m_currentWorkspace(0)
    , m_wmInter(new wm("com.deepin.wm", "/com/deepin/wm", QDBusConnection::sessionBus(), this))
    , m_gsettings(new QGSettings("com.deepin.dde.appearance", "", this))
    , m_blurInter(new ImageBlurInter("com.deepin.daemon.Accounts",
                                     "/com/deepin/daemon/ImageBlur",
                                     QDBusConnection::systemBus(), this))
{
    m_blurInter->setSync(false);

    auto updateWorkspace = [this] (int, int to) {
        setCurrentWorkspace(to);
    };

    connect(m_gsettings, &QGSettings::changed, this, &BackgroundManager::updateBackgrounds);
    connect(m_wmInter, &__wm::WorkspaceSwitched, updateWorkspace);
    connect(m_blurInter, &ImageBlurInter::BlurDone, this, &BackgroundManager::onBlurDone);

    updateBackgrounds();
    updateWorkspace(0, 0);
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
    const QString &current = m_backgrounds[m_currentWorkspace];

    const QString &currentPath = QUrl(current).isLocalFile() ? QUrl(current).toLocalFile() : current;
    const QString &sourcePath = QUrl(source).isLocalFile() ? QUrl(source).toLocalFile() : source;

    if (done && QFile::exists(blur) && currentPath == sourcePath)
        emit currentWorkspaceBackgroundChanged(blur);
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
    m_backgrounds = m_gsettings->get("background-uris").toStringList();

    const QString &current = m_backgrounds[m_currentWorkspace];

    QString path = QUrl(current).isLocalFile() ? QUrl(current).toLocalFile() : current;

    path = QFile::exists(path) ? path : DefaultWallpaper;

    const QString &file = m_blurInter->Get(path);

    if (!file.isEmpty())
        emit currentWorkspaceBackgroundChanged(file);
}
