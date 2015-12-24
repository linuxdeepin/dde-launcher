#include "launcherapp.h"
#include "views/launcherframe.h"
#include "Logger.h"
#include "dbusinterface/launcheradaptor.h"
#include "app/global.h"
#include "app/xcb_misc.h"
#include "controller/dbusworker.h"
#include "dialogs/confirmuninstalldialog.h"
#include "widgets/themeappicon.h"
#include "widgets/commandlinemanager.h"
#include <QDBusConnection>
#include <QThread>
#include <QDebug>

QMap<QString, QString> LauncherApp::UnistallAppNames;

LauncherApp::LauncherApp(QObject *parent) : QObject(parent)
{
    m_launcherFrame = new LauncherFrame;
    new LauncherAdaptor(m_launcherFrame);
    QDBusConnection conn = QDBusConnection::sessionBus();
    conn.registerObject(LauncherPathName, m_launcherFrame);

    m_dbusWorker = new DBusWorker;
    m_dbusThread = new QThread;
    m_dbusWorker->moveToThread(m_dbusThread);

    connect(signalManager, SIGNAL(appUninstalled(QString)), this, SLOT(handleUninstall(QString)));
    connect(signalManager, SIGNAL(gtkIconThemeChanged()), qApp, SLOT(quit()));
}

void LauncherApp::show(){
    m_launcherFrame->Show();
    qDebug() << "LauncherApp show";
    m_dbusThread->start();
    emit signalManager->requestData();
}

void LauncherApp::handleUninstall(QString appKey){
    qDebug() << "handleUninstall" << appKey;
    QString appName = dbusController->getLocalItemInfo(appKey).name;
    LauncherApp::UnistallAppNames.insert(appKey, appName);
    QString iconKey = dbusController->getLocalItemInfo(appKey).iconKey;
    ConfirmUninstallDialog d(m_launcherFrame);
    d.setAppKey(appKey);
    d.setIcon(ThemeAppIcon::getIconPixmap(iconKey, LauncherFrame::IconSize, LauncherFrame::IconSize));
    QString message = tr("Are you sure to uninstall %1 ?").arg(appName);
    d.setMessage(message);
    connect(&d, SIGNAL(buttonClicked(int)), this, SLOT(handleButtonClicked(int)));
    emit signalManager->highlightChanged(false);
    d.exec();
    d.deleteLater();
}

void LauncherApp::handleButtonClicked(int buttonId){
    qDebug() << sender() <<buttonId;
    QString appKey = static_cast<ConfirmUninstallDialog*>(sender())->getAppKey();
    emit signalManager->uninstallActionChanged(appKey, buttonId);
}


LauncherApp::~LauncherApp()
{
    m_launcherFrame->deleteLater();
    m_dbusWorker->deleteLater();
    m_dbusThread->deleteLater();
}

void LauncherApp::addCommandOptions()
{
    QCommandLineOption modeOption(QStringList() << "m" << "mode", "swicth mode", "0");
    CommandLineManager::instance()->addOption(modeOption);
    CommandLineManager::instance()->initOptions();
}
