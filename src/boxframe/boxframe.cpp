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

#include <QDebug>
#include <QUrl>
#include <QFile>
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QPaintEvent>

#define WALLPAPER_CACHE_FOLDER "cache/"
static const QString DefaultBackground = "/usr/share/backgrounds/default_background.jpg";

static QString getLocalFile(const QString &file)
{
    const QUrl url(file);
    return url.isLocalFile() ? url.toLocalFile() : url.url();
}

BoxFrame::BoxFrame(QWidget *parent)
    : QLabel(parent)
    , m_currentWorkspace(-1)
    , m_wmInter(new WMinter("com.deepin.wm", "/com/deepin/wm", QDBusConnection::sessionBus(), this))
    , m_blurInter(new ImageBlurInter("com.deepin.daemon.Accounts", "/com/deepin/daemon/ImageBlur", QDBusConnection::systemBus(), this))
    , m_appearanceInter(new AppearanceInter("com.deepin.daemon.Appearance", "/com/deepin/daemon/Appearance", QDBusConnection::sessionBus(), this))
{
    m_blurInter->setSync(false, false);
    m_appearanceInter->setSync(false, false);

    connect(m_wmInter, &__wm::WorkspaceSwitched, this, &BoxFrame::updateBackgrounds);
    connect(m_blurInter, &ImageBlurInter::BlurDone, this, &BoxFrame::onBlurDone);
    connect(m_appearanceInter, &AppearanceInter::Changed, this, [=] (const QString &type, const QString &) {
        if (type == "background") {
            updateBackgrounds();
        }
    });

    /* 判断一个cache能否被加载的条件
     * 完整路径md5后_屏幕分辨率_工作区 组成文件名，先判断分辨率是否合适，
     * 当异步获取到壁纸以后，再判断路径是否一致。
     */

    // 打开cache目录，扫描所有的文件，选择合适的大小
    const QString cache_folder = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).first();
    QDir dir(QString("%1/%2").arg(cache_folder).arg(QString(WALLPAPER_CACHE_FOLDER)));

    if (!dir.exists()) {
        qDebug() << dir.mkpath(dir.path());
    }

    dir.setFilter(QDir::Files);
    QFileInfoList list = dir.entryInfoList();
    // 获取分辨率
    const std::pair<uint, uint> re = resolution();

    for (auto it = list.constBegin(); it != list.constEnd(); ++it) {
        // hash_resolution is filename
        const QStringList           file_name_analysis = it->fileName().split("_");
        const std::pair<uint, uint> file_re = resolutionByQString(file_name_analysis[1]);
        // check resolution
        if (file_re.second != re.second) continue;

        QFile file(QString("%1/%2").arg(dir.path()).arg(it->fileName()));
        if (!file.open(QIODevice::ReadOnly)) continue;

        QDataStream stream(&file);
        QPixmap     pixmap(QSize(re.first, re.second));
        stream >> pixmap;

        if (pixmap.isNull()) continue;

        // update pixmap
        m_pixmap = pixmap;
        m_cache = backgroundPixmap();

        m_currentBackgroundInfo.Hash = file_name_analysis[0];
        m_currentBackgroundInfo.Resolution = file_name_analysis[1];
        m_currentBackgroundInfo.Workspace = file_name_analysis[2].toUInt();

        // m_background = m_currentBackgroundInfo;
        break;
    }

    QTimer::singleShot(0, this, &BoxFrame::updateBackgrounds);
}

BoxFrame::~BoxFrame()
{
}

// Message for maintainers: DON'T use QSS to set the background !

// This function takes ~2ms by setting QSS to set backgrounds, but causes show() of
// ShutdownFrame takes ~260ms to complete. On the other hand, this function takes
// ~130ms by setting pixmap, yet takes only ~12ms to complete the show() of ShutdownFrame.
// It'll be more obvious on dual screens environment.
void BoxFrame::setBackground(const QString &url)
{
    QPixmap pix(url);

    if (pix.isNull()) {
        pix.load(DefaultBackground);
    }

    m_pixmap = pix;

    updateBackground();
}

const QPixmap BoxFrame::backgroundPixmap() {
    const QSize &size = qApp->primaryScreen()->size() * qApp->primaryScreen()->devicePixelRatio();

    QPixmap cache = m_pixmap.scaled(size,
                                    Qt::KeepAspectRatioByExpanding,
                                    Qt::SmoothTransformation);

    QRect copyRect((cache.width() - size.width()) / 2,
                   (cache.height() - size.height()) / 2,
                   size.width(), size.height());

    cache = cache.copy(copyRect);
    cache.setDevicePixelRatio(devicePixelRatioF());

    return cache;
}

void BoxFrame::updateBackground()
{
    m_cache = backgroundPixmap();

    // saveCurrentPixmap(m_cache);

    updateGradient();

    update();
}

void BoxFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QScreen const *s = qApp->primaryScreen();
    const QRect &geom = s->geometry();
    QRect tr(QPoint(0, 0), geom.size());

    painter.drawPixmap(tr,
                       m_cache,
                       QRect(tr.topLeft(),
                             tr.size() * m_cache.devicePixelRatioF()));
}

void BoxFrame::saveCurrentPixmap(const QPixmap &pixmap)
{
    const QString cache_folder = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).first();
    QDir dir(QString("%1/%2").arg(cache_folder).arg(WALLPAPER_CACHE_FOLDER));

    // create file name
    QString file_path = QString("%1/%2_%3")
                            .arg(dir.path())
                            .arg(QString(QCryptographicHash::hash(m_background.toUtf8(),
                                                                  QCryptographicHash::Md5)
                                             .toHex()))
                            .arg(resolutionToQString(resolution()));

    QFile file(file_path);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream stream(&file);
        stream << pixmap;
        file.close();
    }
}

void BoxFrame::onBlurDone(const QString &source, const QString &blur, bool done)
{
    if (m_currentWorkspace == -1) {
        m_background = blur;
    }
    else {
        const QString &currentPath = QUrl(source).isLocalFile() ? QUrl(source).toLocalFile() : source;
        const QString &sourcePath = QUrl(source).isLocalFile() ? QUrl(source).toLocalFile() : source;

        if (done && QFile::exists(blur) && currentPath == sourcePath) {
            m_background = blur;
        }
    }

    setBackground(m_background);
}

void BoxFrame::updateBackgrounds()
{
    QString path = getLocalFile(m_wmInter->GetCurrentWorkspaceBackground());

    path = QFile::exists(path) ? path : DefaultBackground;

    const QString &file = m_blurInter->Get(path);

    m_background = file.isEmpty() ? path : file;

    setBackground(m_background);
}

const std::pair<uint, uint> BoxFrame::resolution() const
{
    QScreen const *screen { qApp->primaryScreen() };

    const QSize size { screen->size() };

    return std::pair<uint, uint>(size.width(), size.height());
}

const QString BoxFrame::resolutionToQString(const std::pair<uint, uint> resolution) const
{
    return QString("%1x%2").arg(resolution.first).arg(resolution.second);
}

const std::pair<uint, uint> BoxFrame::resolutionByQString(const QString &resolution) const
{
    const QStringList list =  resolution.split("x");

    return std::pair<uint, uint>(list[0].toUInt(), list[1].toUInt());
}
