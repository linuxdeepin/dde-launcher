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

#ifndef UTIL_H
#define UTIL_H

#include "iteminfo.h"

#include <QtCore>
#include <QGSettings>

enum CacheType {
    TextType,
    ImageType
};

QString getQssFromFile(QString filename);
QString joinPath(const QString& path, const QString& fileName);
QString getThumbnailsPath();
QString getCategoryNames(QString text);
const QPixmap loadSvg(const QString &fileName, const int size);
const QPixmap loadSvg(const QString &fileName, const QSize &size);
const QPixmap loadIco(const QString &fileName, const int size);
const QPixmap renderSVG(const QString &path, const QSize &size);
QGSettings *SettingsPtr(const QString &schema_id, const QByteArray &path = QByteArray(), QObject *parent = nullptr);
QGSettings *ModuleSettingsPtr(const QString &module, const QByteArray &path = QByteArray(), QObject *parent = nullptr);
QString qtify_name(const char *name);
QVariant SettingValue(const QString &schema_id, const QByteArray &path = QByteArray(), const QString &key = QString(), const QVariant &fallback = QVariant());
bool createCalendarIcon(const QString &fileName);
int perfectIconSize(const int size);
QString cacheKey(const ItemInfo &itemInfo, CacheType type);
bool getThemeIcon(QPixmap &pixmap, const ItemInfo &itemInfo, const int size, bool reObtain);
QIcon getIcon(const QString &name);
#endif // UTIL_H

