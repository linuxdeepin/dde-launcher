#include "iconcachemanager.h"
#include "appsmanager.h"
#include "util.h"
#include "calculate_util.h"

#include "dbuslauncher.h"

#include <QPixmap>
#include <QIcon>
#include <QString>

QHash<QPair<QString, int>, QVariant> IconCacheManager::m_iconCache = QHash<QPair<QString, int>, QVariant>();
QReadWriteLock IconCacheManager::m_iconLock;

std::atomic<bool> IconCacheManager::m_loadState;
static QList<int> sizeList = { 16, 18, 24, 32, 64, 96, 128, 256 };

IconCacheManager *IconCacheManager::instance()
{
    static IconCacheManager instance;
    return &instance;
}

IconCacheManager::IconCacheManager(QObject *parent)
    : QObject(parent)
    , m_launcherInter(new DBusLauncher(this))
    , m_iconValid(true)
    , m_tryNums(0)
    , m_tryCount(0)
    , m_date(QDate::currentDate())
{
    setIconLoadState(false);
}

void IconCacheManager::createPixmap(const ItemInfo_v1 &itemInfo, int size)
{
    const int iconSize = perfectIconSize(size);
    QPair<QString, int> tmpKey { cacheKey(itemInfo), iconSize};
    if (itemInfo.m_iconKey == "dde-calendar")
        m_calendarInfo = itemInfo;

    if (existInCache(tmpKey))
        return;

    QPixmap pixmap;
    m_iconValid = getThemeIcon(pixmap, itemInfo, size, !m_iconValid);
    if (m_iconValid) {
        m_tryNums = 0;
    } else {
        if (m_tryNums < 10) {
            ++m_tryNums;
            if (!QFile::exists(itemInfo.m_iconKey))
                QIcon::setThemeSearchPaths(QIcon::themeSearchPaths());

            QThread::msleep(10);
            m_iconValid = getThemeIcon(pixmap, itemInfo, size, true);
        } else {
            if (m_tryCount > 10) {
                m_tryCount = 0;

                // 当desktop文件中Icon字段为空，不存在该字段或者字段内容错误时，
                // 直接将齿轮写入缓存，避免显示为空
                QIcon icon = QIcon(":/widgets/images/application-x-desktop.svg");
                const qreal ratio = qApp->devicePixelRatio();
                pixmap = icon.pixmap(QSize(iconSize, iconSize) * ratio);
                pixmap.setDevicePixelRatio(ratio);
                IconCacheManager::insertCache(tmpKey, pixmap);
                return;
            }

            ++m_tryCount;
            QThread::msleep(500);
            m_iconValid = getThemeIcon(pixmap, itemInfo, size, true);
        }
    }
}

double IconCacheManager::getCurRatio()
{
    return SettingValue("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", "apps-icon-ratio", 0.6).toDouble();
}

bool IconCacheManager::existInCache(const QPair<QString, int> &tmpKey)
{
    std::atomic<bool> exist;

    m_iconLock.lockForRead();
    exist.store(m_iconCache.contains(tmpKey) && !m_iconCache[tmpKey].value<QPixmap>().isNull());
    m_iconLock.unlock();

    return exist;
}

void IconCacheManager::getPixFromCache(QPair<QString, int> &tmpKey, QPixmap &pix)
{
    m_iconLock.lockForRead();
    pix = m_iconCache[tmpKey].value<QPixmap>();
    m_iconLock.unlock();
}

/**获取小窗口的资源
 * @brief IconCacheManager::loadWindowIcon
 */
void IconCacheManager::loadWindowIcon()
{
    setIconLoadState(false);

    // 小窗口模式
    const ItemInfoList_v1 &itemList = AppsManager::instance()->windowedFrameItemInfoList();
    for (int i = 0; i < itemList.size(); i++) {
        const ItemInfo_v1 &info = itemList.at(i);
        createPixmap(info, DLauncher::APP_ITEM_ICON_SIZE);
    }

    setIconLoadState(true);
    emit iconLoaded();
}

/**小窗口显示后加载的资源
 * @brief IconCacheManager::loadOtherIcon
 */
void IconCacheManager::loadOtherIcon()
{
    // 小窗口模式卸载,拖拽图标
    const ItemInfoList_v1 &itemList = AppsManager::instance()->windowedFrameItemInfoList();
    for (int i = 0; i < itemList.size(); i++) {
        const ItemInfo_v1 &info = itemList.at(i);
        createPixmap(info, DLauncher::APP_DLG_ICON_SIZE);
        createPixmap(info, DLauncher::APP_DRAG_ICON_SIZE);
    }
}

void IconCacheManager::loadItem(const ItemInfo_v1 &info, const QString &operationStr)
{
    if (operationStr == "updated" || operationStr == "deleted")
        removeItemFromCache(info);

    // 小窗口
    createPixmap(info, DLauncher::APP_ITEM_ICON_SIZE);

    // 全屏自由
    int appSize = CalculateUtil::instance()->calculateIconSize(ALL_APPS);

    double ratio = CalculateUtil::instance()->getCurRatio();
    int iconWidth = (appSize * ratio);
    createPixmap(info, iconWidth);
}

void IconCacheManager::loadCurRatioIcon(int mode)
{
    const ItemInfoList_v1 &itemList = AppsManager::instance()->fullscreenItemInfoList();
    int appSize = CalculateUtil::instance()->calculateIconSize(mode);

    for (int i = 0; i < itemList.size(); i++) {
        const ItemInfo_v1 &info = itemList.at(i);
        createPixmap(info, appSize * getCurRatio());
    }

    setIconLoadState(true);
    emit iconLoaded();
}

void IconCacheManager::loadOtherRatioIcon(int mode)
{
    int appSize = CalculateUtil::instance()->calculateIconSize(mode);
    const ItemInfoList_v1 &itemList = AppsManager::instance()->fullscreenItemInfoList();

    double ratio = CalculateUtil::instance()->getCurRatio();
    for (int i = 0; i < itemList.size(); i++) {
        const ItemInfo_v1 &info = itemList.at(i);
        createPixmap(info, appSize * ratio);
    }
}

/** 图标主题变化时，加载全屏资源
 * @brief IconCacheManager::loadFullWindowIcon
 */
void IconCacheManager::loadFullWindowIcon()
{
    loadCurRatioIcon(ALL_APPS);
    loadOtherRatioIcon(ALL_APPS);
}

void IconCacheManager::insertCache(const QPair<QString, int> &tmpKey, const QPixmap &pix)
{
    m_iconLock.lockForWrite();
    if (!m_iconCache.contains(tmpKey) || (m_iconCache.contains(tmpKey) && m_iconCache[tmpKey].value<QPixmap>().isNull()))
        m_iconCache[tmpKey] = pix;

    m_iconLock.unlock();
}

void IconCacheManager::removeItemFromCache(const ItemInfo_v1 &info)
{
    for (int i = 0; i < sizeList.size(); i++) {
        QPair<QString, int> pixKey { cacheKey(info), sizeList.at(i) };
        if (existInCache(pixKey)) {
            m_iconLock.lockForWrite();
            m_iconCache.remove(pixKey);
            m_iconLock.unlock();
        }
    }
}

void IconCacheManager::resetIconData()
{
    // 清缓存
    m_iconLock.lockForWrite();
    m_iconCache.clear();
    m_iconLock.unlock();

    // 重置状态
    setIconLoadState(false);
}

void IconCacheManager::removeSmallWindowCache()
{
    auto removeCache = [](const ItemInfoList_v1 &itemList, int pixSize) {
        for (int i = 0; i < itemList.size(); i++) {
            const ItemInfo_v1 &itemInfo = itemList.at(i);
            const int iconSize = perfectIconSize(pixSize);
            QPair<QString, int> pixKey { cacheKey(itemInfo), iconSize };

            if (existInCache(pixKey)) {
                m_iconLock.lockForWrite();
                m_iconCache.remove(pixKey);
                m_iconLock.unlock();
            }
        }
    };


    // 小窗口应用图标
    removeCache(AppsManager::instance()->windowedFrameItemInfoList(), DLauncher::APP_ITEM_ICON_SIZE);
    setIconLoadState(false);
}

bool IconCacheManager::iconLoadState()
{
    return m_loadState.load();
}

void IconCacheManager::setIconLoadState(bool state)
{
    m_loadState.store(state);
}

void IconCacheManager::updateCanlendarIcon()
{
    if (m_date != QDate::currentDate()) {
        removeItemFromCache(m_calendarInfo);

        for (int i = 0; i < sizeList.size(); i++)
            createPixmap(m_calendarInfo, sizeList.at(i));

        // 刷新界面
        emit iconLoaded();
        m_date = QDate::currentDate();
    }
}
