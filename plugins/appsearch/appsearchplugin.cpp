#include "appsearchplugin.h"

AppSearchPlugin::AppSearchPlugin(QObject *parent)
   : QObject (parent)
{
}

void AppSearchPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInterface = proxyInter;

    // 通知启动器界面显示应用占位
    // m_proxyInterface->itemAdded(this, AppInfo);

    // 通知启动器界面移除应用占位
    // m_proxyInterface->itemRemoved(this, AppInfo);

    // 通知启动器更新进度条等数据...
    // 第三方调用 m_proxyInterface->itemUpdated(this, AppInfo);
}

const QString AppSearchPlugin::pluginName() const
{
    return QString("appsearch");
}

const QString AppSearchPlugin::pluginDisplayName()
{
    return QString(tr("Native Result:"));
}

AppInfoList AppSearchPlugin::search(const QString &searchText)
{
    Q_UNUSED(searchText);

#ifdef QT_DEBUG
    AppInfoList appList;
    AppInfo list;
    list.m_desktop = "/usr/share/applications/com.youdao.cidian.desktop";
    list.m_name = "youdao-dict";
    list.m_key = "com.youdao.cidian";
    list.m_iconKey = "youdao-dict";
    list.m_categoryId = 61649644891;
    list.m_progressValue = 50;
    list.m_status = 1;
    list.m_description = "Your searched apps in the appstore:";
    appList << list;
    return appList;
#else
    return AppInfoList();
#endif
}
