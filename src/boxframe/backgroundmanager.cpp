// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backgroundmanager.h"
#include "appsmanager.h"
#include "util.h"

#include <QApplication>
#include <QtConcurrent>

using namespace com::deepin;

const QString DefaultWallpaper = "/usr/share/backgrounds/default_background.jpg";
const QString DisplayInterface("com.deepin.daemon.Display");

static QString getLocalFile(const QString &file)
{
    const QUrl url(file);
    return url.isLocalFile() ? url.toLocalFile() : url.url();
}

DisplayHelper::DisplayHelper(QObject *parent)
    : QObject(parent)
    , m_displayInter(new DisplayInter("com.deepin.daemon.Display", "/com/deepin/daemon/Display", QDBusConnection::sessionBus(), this))
{
    m_displayMode = m_displayInter->GetRealDisplayMode();

    connect(m_displayInter, &DisplayInter::DisplayModeChanged, this, &DisplayHelper::updateDisplayMode);
    connect(m_displayInter, &DisplayInter::PrimaryChanged, this, &DisplayHelper::updateDisplayMode);
}

void DisplayHelper::updateDisplayMode()
{
    m_displayMode = m_displayInter->GetRealDisplayMode();
}

BackgroundManager::BackgroundManager(QObject *parent)
    : QObject(parent)
    , m_currentWorkspace(-1)
    , m_wmInter(new wm("com.deepin.wm", "/com/deepin/wm", QDBusConnection::sessionBus(), this))
    , m_imageEffectInter(new ImageEffectInter("com.deepin.daemon.ImageEffect", "/com/deepin/daemon/ImageEffect", QDBusConnection::systemBus(), this))
    , m_imageblur(new ImageEffeblur("com.deepin.daemon.ImageEffect", "/com/deepin/daemon/ImageBlur", QDBusConnection::systemBus(), this))
    , m_appearanceInter(new AppearanceInter("com.deepin.daemon.Appearance", "/com/deepin/daemon/Appearance", QDBusConnection::sessionBus(), this))
    , m_displayInter(new DisplayInter(DisplayInterface, "/com/deepin/daemon/Display", QDBusConnection::sessionBus(), this))
    , m_fileName(QString())
{
    m_appearanceInter->setSync(false, false);

    m_displayMode = m_displayInter->GetRealDisplayMode();

    connect(m_wmInter, &__wm::WorkspaceSwitched, this, &BackgroundManager::updateBlurBackgrounds);
    connect(m_wmInter, &__wm::WorkspaceBackgroundChanged, this, &BackgroundManager::updateBlurBackgrounds);
    connect(m_appearanceInter, &AppearanceInter::Changed, this, &BackgroundManager::onAppearanceChanged);
    connect(m_displayInter, &DisplayInter::DisplayModeChanged, this, &BackgroundManager::onDisplayModeChanged);
    connect(m_displayInter, &DisplayInter::PrimaryChanged, this, &BackgroundManager::onPrimaryChanged);
    connect(m_displayInter, &DisplayInter::DisplayModeChanged, this, &BackgroundManager::updateBlurBackgrounds);

    connect(m_imageblur, &ImageEffeblur::BlurDone, this, &BackgroundManager::onGetBlurImageFromDbus);

    updateBlurBackgrounds();
}

void BackgroundManager::getImageDataFromDbus(const QString &filePath)
{
    // 异步获取模糊以及pixmix算法处理后的桌面背景(分类模式的视图背景)
    QFutureWatcher<QString> *imageEffectWatcher = new QFutureWatcher<QString>(this);
    connect(imageEffectWatcher, &QFutureWatcher<QString>::finished, this, [this, imageEffectWatcher]{
        imageEffectWatcher->deleteLater();
        m_blurBackground = imageEffectWatcher->result();
        if (!m_blurBackground.isEmpty())
            emit currentWorkspaceBlurBackgroundChanged(m_blurBackground);
    });
    QFuture<QString> imageblurFuture = QtConcurrent::run([this, filePath]() ->QString {
        if (m_imageblur.isNull())
            return filePath;

        QDBusPendingReply<QString> blurReply = m_imageblur->Get(filePath);
        blurReply.waitForFinished();
        if (m_imageEffectInter.isNull())
            return filePath;

        // 处理完会触发BlurDone信号,总之 imageblurFuture 必须得有返回值,否则会导致imageEffectWatcher->result()获取不到值导致异常
        QDBusPendingReply<QString> effectInterReply = m_imageEffectInter->Get("", blurReply.value());
        effectInterReply.waitForFinished();
        if (effectInterReply.isError())
            return filePath;

        return effectInterReply.value();
    });
    imageEffectWatcher->setFuture(imageblurFuture);

    // 异步获取全屏桌面背景
    QFutureWatcher<QString> *effectInterWatcher = new QFutureWatcher<QString> (this);
    connect(effectInterWatcher, &QFutureWatcher<QString>::finished, this, [this, effectInterWatcher](){
        effectInterWatcher->deleteLater();
        m_background = effectInterWatcher->result();
        if (!m_background.isEmpty())
            emit currentWorkspaceBackgroundChanged(m_background);
    });
    QFuture<QString> effectInterFuture = QtConcurrent::run([this, filePath]() ->QString {
        if (!m_imageEffectInter)
            return filePath;

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
    const QScreen *screen = AppsManager::instance()->currentScreen();
    if (!screen)
        return;

    QString screenName = screen->name();

    if (isWaylandDisplay()) {
        const QList<QDBusObjectPath> monitorList = m_displayInter->monitors();
        for (int i = 0; i < monitorList.size(); i++) {
            DisplayMonitor monitor(DisplayInterface, QString("%1").arg(monitorList.at(i).path()), QDBusConnection::sessionBus(), this);
            // wayland下 正常接入主机且显示, 当屏幕左上角坐标一致时, 获取屏幕名称
            if ((monitor.enabled() == true) && (monitor.x() == screen->geometry().x()) && (monitor.y() == screen->geometry().y())) {
                screenName = monitor.name();
                break;
            }
        }
    }

    QString path = getLocalFile(m_wmInter->GetCurrentWorkspaceBackgroundForMonitor(screenName));
    m_fileName = QFile::exists(path) ? path : DefaultWallpaper;

    getImageDataFromDbus(m_fileName);
}

void BackgroundManager::onAppearanceChanged(const QString &type, const QString &str)
{
    Q_UNUSED(str);

    if (type == "background")
        updateBlurBackgrounds();
}

void BackgroundManager::onDisplayModeChanged(uchar value)
{
    Q_UNUSED(value);

    m_displayMode = m_displayInter->GetRealDisplayMode();
}

void BackgroundManager::onPrimaryChanged(const QString &value)
{
    Q_UNUSED(value);

    m_displayMode = m_displayInter->GetRealDisplayMode();
    updateBlurBackgrounds();
}

/**获取分类模式的视图模糊背景图片路径
 * @brief BackgroundManager::onGetBlurImageFromDbus
 * @param file 本地背景图片文件名称
 * @param blurFile 模糊处理后的文件名称
 * @param status 模糊处理成功返回true,否则返回false
 */
void BackgroundManager::onGetBlurImageFromDbus(const QString &file, const QString &blurFile, bool status)
{
    // 信号返回的文件路径与本地获取的文件路径比对
    if (status && file == m_fileName) {
        // 异步获取pixmix处理模糊后的背景(分类模式的视图背景)
        QFutureWatcher<QString> *imageEffectWatcher = new QFutureWatcher<QString>(this);
        connect(imageEffectWatcher, &QFutureWatcher<QString>::finished, this, [this, imageEffectWatcher]{
            imageEffectWatcher->deleteLater();
            m_blurBackground = imageEffectWatcher->result();
            if (!m_blurBackground.isEmpty())
                emit currentWorkspaceBlurBackgroundChanged(m_blurBackground);
        });

        // 按照dde-session-ui/dde-pixmix 的算法处理
        QFuture<QString> imageblurFuture = QtConcurrent::run([this, blurFile]() ->QString {
            QDBusPendingReply<QString> effectInterReply = m_imageEffectInter->Get("", blurFile);
            effectInterReply.waitForFinished();

            if (effectInterReply.isError())
                return blurFile;

            return effectInterReply.value();
        });

        imageEffectWatcher->setFuture(imageblurFuture);
    }
}
