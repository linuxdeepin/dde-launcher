#ifndef UTIL_H
#define UTIL_H

#include <QtCore>

QString getQssFromFile(QString filename);
QString joinPath(const QString& path, const QString& fileName);
QString getThumbnailsPath();
#endif // UTIL_H

