#ifndef MENUCONTROLLER_H
#define MENUCONTROLLER_H


#include <QtCore>

class MenumanagerInterface;
class MenuInterface;
class DBusDockedAppManager;
class QJsonObject;

#define MenuManager_service "com.deepin.menu"
#define MenuManager_path "/com/deepin/menu"


class MenuController : public QObject
{
    Q_OBJECT
public:

    explicit MenuController(QObject *parent = 0);
    ~MenuController();

    void initConnect();
    bool isItemOnDock(QString appKey);
    bool isItemOnDesktop(QString appKey);
    bool isItemStartup(QString appKey);

    QJsonObject createMenuItem(int itemId, QString itemText);
    QJsonObject createSeperator();

    QString createMenuContent(QString appKey);
    QString registerMenu();
    QString JsonToQString(QPoint pos, QString menucontent);

    void showMenu(QString menuDBusObjectPath, QString menuContent);

signals:

public slots:
    void showMenuByAppItem(QString appKey, QPoint pos);
    void menuItemInvoked(QString itemId, bool flag);

    void handleOpen(QString appKey);
    void handleToDesktop(QString appkey);
    void handleToDock(QString appKey);
    void handleToStartup(QString appKey);

    void handleUninstall(QString appKey);
    void handleUninstallAction(int id);
    void startUnistall(QString appKey);
    void handleUninstallSuccess(const QString& appKey);
    void handleUninstallFail(const QString& appKey, const QString& message);

private:
    MenumanagerInterface* m_menuManagerInterface;
    MenuInterface* m_menuInterface;
    DBusDockedAppManager* m_dockAppManagerInterface;

    QString m_appKeyRightClicked;
    bool m_isItemOnDesktop;
    bool m_isItemOnDock;
    bool m_isItemStartup;
};

#endif // MENUCONTROLLER_H
