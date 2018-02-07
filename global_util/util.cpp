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

QString getQssFromFile(QString filename)
{
    QFile f(filename);
    QString qss = "";
    if (f.open(QFile::ReadOnly))
    {
        qss = QLatin1String(f.readAll());
        f.close();
    }
    return qss;
}

QString joinPath(const QString& path, const QString& fileName){
    QString separator(QDir::separator());
    return QString("%1%2%3").arg(path, separator, fileName);
}

QString getThumbnailsPath(){
    QString cachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).at(0);
    QString thumbnailPath = joinPath(cachePath, "thumbnails");
    if (!QDir(thumbnailPath).exists()){
        QDir(thumbnailPath).mkpath(thumbnailPath);
    }
    return thumbnailPath;
}

const QPixmap loadSvg(const QString &fileName, const int size)
{
    QPixmap pixmap(size, size);
    QSvgRenderer renderer(fileName);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    return pixmap;
}
