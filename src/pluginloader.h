#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <QObject>

class PluginLoader : public QObject
{
    Q_OBJECT
public:
    explicit PluginLoader(QObject *parent = nullptr);
};

#endif // PLUGINLOADER_H
