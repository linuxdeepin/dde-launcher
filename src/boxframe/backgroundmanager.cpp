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
#include "../global_util/constants.h"

using namespace com::deepin;

static const QString DefaultWallpaper = "/usr/share/backgrounds/default_background.jpg";
static const QString DefaultWallpaper2 = "/usr/share/wallpapers/deepin/desktop.bmp";

static QString getLocalFile(const QString &file)
{
    const QUrl url(file);
    return url.isLocalFile() ? url.toLocalFile() : url.url();
}

BackgroundManager::BackgroundManager(QObject *parent)
    : QObject(parent)
    , m_currentWorkspace(-1)
    , m_wmInter(new wm("com.deepin.wm", "/com/deepin/wm", QDBusConnection::sessionBus(), this))
    , m_imageEffectInter(new ImageEffectInter("com.deepin.daemon.ImageEffect", "/com/deepin/daemon/ImageEffect", QDBusConnection::systemBus(), this))
    , m_appearanceInter(new AppearanceInter("com.deepin.daemon.Appearance", "/com/deepin/daemon/Appearance", QDBusConnection::sessionBus(), this))
{
    m_appearanceInter->setSync(false, false);

    connect(m_wmInter, &__wm::WorkspaceSwitched, this, &BackgroundManager::updateBackgrounds);
    connect(m_appearanceInter, &AppearanceInter::Changed, this, [ = ](const QString & type, const QString &) {
        if (type == "background") {
            updateBackgrounds();
        }
    });

    QTimer::singleShot(0, this, &BackgroundManager::updateBackgrounds);
}

void BackgroundManager::updateBackgrounds()
{
    QString path = getLocalFile(m_wmInter->GetCurrentWorkspaceBackground());

    QString filePath = QFile::exists(path) ? path : DefaultWallpaper;

    m_background = m_imageEffectInter->Get("", filePath);

    emit currentWorkspaceBackgroundChanged(m_background);
}
