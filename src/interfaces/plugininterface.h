#pragma once

#include <QVariant>
#include <QSharedPointer>
#include <QtConcurrent>

class AppData;

class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual void init() {}
    virtual void destory() {}
    virtual QString name() { return {}; }
    virtual QString description() { return {}; }
    virtual int count() { return 0; }

    /*
     * \~chinese \brief 返回对应元素的数据
     */
    virtual QSharedPointer<AppData> data(int index) {
        Q_UNUSED(index);
        return nullptr;
    }

    /*
     *  \~chinese \brief 必须重载该方法，插件需要提供运行方法并强制要求异步调用。
     */
    virtual QFuture<bool> run(QSharedPointer<AppData> app) = 0;
};

QT_BEGIN_NAMESPACE
#define ModuleInterface_iid "com.deepin.dde.launcher.IPlugin"

Q_DECLARE_INTERFACE(IPlugin, ModuleInterface_iid)
QT_END_NAMESPACE
