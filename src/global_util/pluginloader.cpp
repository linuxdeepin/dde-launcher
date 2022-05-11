/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
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

#include "pluginloader.h"

#include <QDir>
#include <QDebug>
#include <QLibrary>
#include <QApplication>

#include <DSysInfo>

DCORE_USE_NAMESPACE

PluginLoader::PluginLoader(QObject *parent)
    : QThread(parent)
{
}

void PluginLoader::run()
{
    QString pluginsDirPath(qApp->applicationDirPath() + "/plugins");
#ifndef QT_DEBUG
    pluginsDirPath = "/usr/lib/dde-launcher/plugins";
#endif
    qInfo() << "using launcher plugins dir:" << pluginsDirPath;

    QDir pluginsDir(pluginsDirPath);

    const QStringList files = pluginsDir.entryList(QDir::Files);
    QStringList plugins;

    // 查找可用插件
    for (QString file : files) {
        if (!QLibrary::isLibrary(file))
            continue;

        plugins << file;
    }

    for (auto plugin : plugins) {
        emit pluginFounded(pluginsDir.absoluteFilePath(plugin));
    }

    emit finished();
}
