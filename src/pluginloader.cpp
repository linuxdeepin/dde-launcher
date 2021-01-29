#include "pluginloader.h"

#include "interfaces/plugininterface.h"

#include <QLibrary>
#include <QDir>
#include <QtConcurrent>
#include <QJsonObject>

PluginLoader::PluginLoader(QObject *parent) : QObject(parent)
{
    QDir pluginsDir("/usr/lib/dde-launcher/plugins");
    const QStringList files = pluginsDir.entryList(QDir::Files);

    for (const QString &path : files) {
        const QString &pluginPath = pluginsDir.absoluteFilePath(path);
        if (!QLibrary::isLibrary(pluginPath)) {
            continue;
        }

        QPluginLoader *pluginLoader = new QPluginLoader(pluginPath);
        const QJsonObject &meta = pluginLoader->metaData().value("MetaData").toObject();
        const QString &pluginApi = meta.value("api").toString();

        IPlugin* plugin = qobject_cast<IPlugin*>(pluginLoader->instance());
        if (!plugin) {
            const QString& error = "load plugin failed. " + pluginLoader->errorString() + " " + pluginPath;
            qErrnoWarning(error.toUtf8());
            pluginLoader->unload();
            pluginLoader->deleteLater();
            continue;
        }

        qInfo() << "load plugin: " << plugin->name();
        qInfo() << "plugin description: " << plugin->description();
    }
}
