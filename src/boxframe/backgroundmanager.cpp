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

#include <QApplication>
#include <QtConcurrent>

using namespace com::deepin;

static const QString DefaultWallpaper = "/usr/share/backgrounds/default_background.jpg";

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
    , m_imageblur(new ImageEffeblur("com.deepin.daemon.ImageEffect", "/com/deepin/daemon/ImageBlur", QDBusConnection::systemBus(), this))
    , m_appearanceInter(new AppearanceInter("com.deepin.daemon.Appearance", "/com/deepin/daemon/Appearance", QDBusConnection::sessionBus(), this))
    , m_displayInter(new DisplayInter("com.deepin.daemon.Display", "/com/deepin/daemon/Display", QDBusConnection::sessionBus(), this))
{
    m_appearanceInter->setSync(false, false);

    m_displayMode = m_displayInter->GetRealDisplayMode();

    connect(m_wmInter, &__wm::WorkspaceSwitched, this, &BackgroundManager::updateBlurBackgrounds);
    connect(m_wmInter, &__wm::WorkspaceBackgroundChanged, this, &BackgroundManager::updateBlurBackgrounds);
    connect(m_appearanceInter, &AppearanceInter::Changed, this, [ = ](const QString & type, const QString &) {
        if (type == "background") {
            updateBlurBackgrounds();
        }
    });
    connect(m_displayInter, &DisplayInter::DisplayModeChanged, this, [=](uchar) {
        m_displayMode = m_displayInter->GetRealDisplayMode();
    });

    connect(m_displayInter, &DisplayInter::DisplayModeChanged, this, &BackgroundManager::updateBlurBackgrounds);

    updateBlurBackgrounds();
}

void BackgroundManager::getImageDataFromDbus(const QString &filePath)
{
    // get BlurBackground async
    QFutureWatcher<QString> *imageEffectWatcher = new QFutureWatcher<QString>(this);
    connect(imageEffectWatcher, &QFutureWatcher<QString>::finished, this, [this, imageEffectWatcher]{
        imageEffectWatcher->deleteLater();
        m_blurBackground = imageEffectWatcher->result();
        if ("" != m_blurBackground && !m_blurBackground.isEmpty())
            emit currentWorkspaceBlurBackgroundChanged(m_blurBackground);
    });
    QFuture<QString> imageblurFuture = QtConcurrent::run([this, filePath]() ->QString {
        QDBusPendingReply<QString> blurReply = m_imageblur->Get(filePath);
        blurReply.waitForFinished();
        if (blurReply.value() != "")
        {
            QDBusPendingReply<QString> effectInterReply = m_imageEffectInter->Get("", blurReply.value());
            effectInterReply.waitForFinished();
            if (effectInterReply.isError()) {
                qWarning() << "ImageEffeblur Get error:" << effectInterReply.error();
                return filePath;
            }
            return effectInterReply.value();
        }

        return filePath;
    });
    imageEffectWatcher->setFuture(imageblurFuture);

    // get Background async
    QFutureWatcher<QString> *effectInterWatcher = new QFutureWatcher<QString> (this);
    connect(effectInterWatcher, &QFutureWatcher<QString>::finished, this, [this, effectInterWatcher](){
        effectInterWatcher->deleteLater();
        m_background = effectInterWatcher->result();
        if ("" != m_background && !m_background.isEmpty())
            emit currentWorkspaceBackgroundChanged(m_background);
    });
    QFuture<QString> effectInterFuture = QtConcurrent::run([this, filePath]() ->QString {
        QDBusPendingReply<QString> effectInterReply = m_imageEffectInter->Get("", filePath);
        effectInterReply.waitForFinished();
        if (effectInterReply.isError()) {
            qWarning() << "ImageEffeblur Get error:" << effectInterReply.error();
            return filePath;
        }
        return effectInterReply.value();
    });
    effectInterWatcher->setFuture(effectInterFuture);
}

void BackgroundManager::updateBlurBackgrounds()
{
    QString screenName = qApp->primaryScreen()->name();

    if (m_displayMode != MERGE_MODE) {
        QWidget *parentWidget =qobject_cast<QWidget *>(parent());
        QDesktopWidget *desktopwidget = QApplication::desktop();
        int screenIndex = desktopwidget->screenNumber(parentWidget);
        QList<QScreen *> screens = qApp->screens();
        screenName = screens[screenIndex]->name();
    }

    qDebug() << "current screen name:" << screenName;

    QString path = getLocalFile(m_wmInter->GetCurrentWorkspaceBackgroundForMonitor(screenName));
    QString filePath = QFile::exists(path) ? path : DefaultWallpaper;

    getImageDataFromDbus(filePath);
}
