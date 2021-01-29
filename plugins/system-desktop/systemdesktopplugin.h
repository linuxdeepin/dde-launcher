                                                                                      #ifndef SYSTEMDESKTOPPLUGIN_H
#define SYSTEMDESKTOPPLUGIN_H

#include "plugininterface.h"
#include <QObject>

class SystemDesktopPlugin : public QObject, IPlugin
{
    Q_OBJECT
    Q_INTERFACES(IPlugin)
    Q_PLUGIN_METADATA(IID "com.deepin.dde.launcher.IPlugin" FILE "plugin.json")
public:
    explicit SystemDesktopPlugin(QObject *parent = nullptr);

    virtual void init() override;
    virtual void destory() override;
    virtual QString name() override;
    virtual QString description() override;
    virtual int count() override;
    virtual QSharedPointer<AppData> data(int index) override;
    virtual QFuture<bool> run(QSharedPointer<AppData> app) override;
};

#endif // SYSTEMDESKTOPPLUGIN_H
