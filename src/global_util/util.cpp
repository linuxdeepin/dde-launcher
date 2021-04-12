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

#include "util.h"

#include <QStandardPaths>
#include <QDir>
#include <QPixmap>
#include <QPainter>
#include <QSvgRenderer>
#include <QImageReader>
#include <QApplication>

QString getQssFromFile(QString filename)
{
    if (!QFileInfo::exists(filename))
        return QString();

    QFile f(filename);
    QString qss = "";
    if (f.open(QFile::ReadOnly))
    {
        qss = QLatin1String(f.readAll());
        f.close();
    }
    return qss;
}

QString joinPath(const QString& path, const QString& fileName)
{
    QString separator(QDir::separator());
    return QString("%1%2%3").arg(path, separator, fileName);
}

QString getThumbnailsPath()
{
    QString cachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).at(0);
    QString thumbnailPath = joinPath(cachePath, "thumbnails");
    if (!QDir(thumbnailPath).exists()){
        QDir(thumbnailPath).mkpath(thumbnailPath);
    }

    return thumbnailPath;
}

const QPixmap loadSvg(const QString &fileName, const int size)
{
    if (!QFileInfo::exists(fileName))
        return QPixmap();

    QPixmap pixmap(size, size);
    QSvgRenderer renderer(fileName);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    renderer.render(&painter);
    painter.end();

    return pixmap;
}

/**
 * @brief renderSVG 根据实体屏幕渲染指定路径、指定大小的图片
 * @param path 渲染图片的路径
 * @param size 渲染图片的大小
 * @return 返回渲染后的pixmap
 */
const QPixmap renderSVG(const QString &path, const QSize &size)
{
    if (!QFileInfo::exists(path))
        return QPixmap();

    QImageReader reader;
    QPixmap pixmap;
    reader.setFileName(path);
    if (reader.canRead()) {
        const qreal ratio = qApp->devicePixelRatio();
        reader.setScaledSize(size * ratio);
        pixmap = QPixmap::fromImage(reader.read());
        pixmap.setDevicePixelRatio(ratio);
    }
    else {
        pixmap.load(path);
    }

    return pixmap;
}

const QPixmap loadSvg(const QString &fileName, const QSize &size)
{
    if (!QFileInfo::exists(fileName))
         return QPixmap();

    QPixmap pixmap(size);
    QSvgRenderer renderer(fileName);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    return pixmap;
}

/**
 * @brief SettingsPtr 根据给定信息返回一个QGSettings指针
 * @param schema_id The id of the schema
 * @param path If non-empty, specifies the path for a relocatable schema
 * @param parent 创建指针的付对象
 * @return 返回QGSetting指针对象
 */
QGSettings *SettingsPtr(const QString &schema_id, const QByteArray &path, QObject *parent)
{
    if (QGSettings::isSchemaInstalled(schema_id.toUtf8())) {
        QGSettings *settings = new QGSettings(schema_id.toUtf8(), path, parent);
        return settings;
    }
    qDebug() << "Cannot find gsettings, schema_id:" << schema_id;
    return nullptr;
}

/**
 * @brief SettingsPtr 根据给定信息返回一个QGSettings指针
 * @param module 传入QGSettings构造函数时，会添加"com.deepin.dde.dock.module."前缀
 * @param path If non-empty, specifies the path for a relocatable schema
 * @param parent 创建指针的付对象
 * @return
 */
QGSettings *ModuleSettingsPtr(const QString &module, const QByteArray &path, QObject *parent)
{
    return SettingsPtr("com.deepin.dde.dock.module." + module, path, parent);
}

/* convert 'some-key' to 'someKey' or 'SomeKey'.
 * the second form is needed for appending to 'set' for 'setSomeKey'
 */
QString qtify_name(const char *name)
{
    bool next_cap = false;
    QString result;

    while (*name) {
        if (*name == '-') {
            next_cap = true;
        } else if (next_cap) {
            result.append(QChar(*name).toUpper().toLatin1());
            next_cap = false;
        } else {
            result.append(*name);
        }

        name++;
    }

    return result;
}

/**
 * @brief SettingValue 根据给定信息返回获取的值
 * @param schema_id The id of the schema
 * @param path If non-empty, specifies the path for a relocatable schema
 * @param key 对应信息的key值
 * @param fallback 如果找不到信息，返回此默认值
 * @return
 */
QVariant SettingValue(const QString &schema_id, const QByteArray &path, const QString &key, const QVariant &fallback)
{
    const QGSettings *settings = SettingsPtr(schema_id, path);

    if (settings && ((settings->keys().contains(key)) || settings->keys().contains(qtify_name(key.toUtf8().data())))) {
        QVariant v = settings->get(key);
        delete settings;
        return v;
    } else {
        qDebug() << "Cannot find gsettings, schema_id:" << schema_id
                 << " path:" << path << " key:" << key
                 << "Use fallback value:" << fallback;
        return fallback;
    }
}
