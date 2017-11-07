/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
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
    if (m_backgrounds.length() >= m_currentWorkspace + 1) {
        return m_backgrounds.at(m_currentWorkspace);
    }

    return DefaultWallpaper;
}

void BackgroundManager::onGetBlurFinished(QDBusPendingCallWatcher *w)
{
    if (!w->isError()) {
        QDBusPendingReply<QString> reply = w->reply();
        const QString &value = reply.value();
        emit currentWorkspaceBackgroundChanged(value.isEmpty() ? currentWorkspaceBackground() : value);
    }

    w->deleteLater();
}

void BackgroundManager::onBlurDone(const QString &source, const QString &blur, bool done)
{
    const QString &current = currentWorkspaceBackground();

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

    const QString &current = currentWorkspaceBackground();

    QString path = QUrl(current).isLocalFile() ? QUrl(current).toLocalFile() : current;

    QDBusPendingReply<QString> call = m_blurInter->Get(path);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &BackgroundManager::onGetBlurFinished);
}
