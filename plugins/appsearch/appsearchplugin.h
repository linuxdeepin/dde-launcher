#ifndef APPSEARCHPLUGIN_H
#define APPSEARCHPLUGIN_H

#include "plugininterface.h"
#include "common.h"

#include <QObject>

class AppSearchPlugin : public QObject, PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)
    Q_PLUGIN_METADATA(IID PluginInterface_IID FILE "appsearch.json")

public:
    explicit AppSearchPlugin(QObject *parent = Q_NULLPTR);
    ~AppSearchPlugin() {}

    virtual void init(PluginProxyInterface *proxyInter);
    virtual const QString pluginName() const;
    virtual const QString pluginDisplayName();
    virtual AppInfoList search(const QString &searchText);
};

#endif
