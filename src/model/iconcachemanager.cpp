#include "iconcachemanager.h"
#include "appsmanager.h"
#include "util.h"
#include "calculate_util.h"

#include "dbuslauncher.h"

#include <QPixmap>
#include <QIcon>
#include <QString>

QHash<QPair<QString, int>, QVariant> IconCacheManager::m_CacheData = QHash<QPair<QString, int>, QVariant>();
QReadWriteLock IconCacheManager::m_cacheDataLock;

std::atomic<bool> IconCacheManager::m_loadState;
std::atomic<bool> IconCacheManager::m_fullFreeLoadState;
std::atomic<bool> IconCacheManager::m_fullCategoryLoadState;
std::atomic<bool> IconCacheManager::m_smallWindowLoadState;

static QList<double> ratioList = { 0.2, 0.3, 0.4, 0.5, 0.6 };

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
    , m_TryCount(0)
{
    setIconLoadState(false);

    // 开始时，未加载到内存
    setFullFreeLoadState(false);
    setFullCategoryLoadState(false);
    setSmallWindowLoadState(false);
}

void IconCacheManager::createPixmap(const ItemInfo &itemInfo, int size)
{
    const int iconSize = perfectIconSize(size);
    QPair<QString, int> tmpKey { cacheKey(itemInfo, CacheType::ImageType), iconSize};

    if (existInCache(tmpKey))
        return;

    QPixmap pixmap;
    m_iconValid = getThemeIcon(pixmap, itemInfo, size, !m_iconValid);
    if (m_iconValid) {
        m_tryNums = 0;
    } else {
        m_itemInfo = itemInfo;
        m_iconSize = size;
        if (m_tryNums < 10) {
            ++m_tryNums;
            if (!QFile::exists(itemInfo.m_iconKey))
                QIcon::setThemeSearchPaths(QIcon::themeSearchPaths());

            QThread::msleep(10);
            m_iconValid = getThemeIcon(pixmap, m_itemInfo, m_iconSize, true);
        } else {
            if (m_TryCount > 10) {
                m_TryCount = 0;
                return;
            }

            ++m_TryCount;
            QThread::msleep(500);
            m_iconValid = getThemeIcon(pixmap, m_itemInfo, m_iconSize, true);
        }
    }
}

bool IconCacheManager::existInCache(const QPair<QString, int> &tmpKey)
{
    std::atomic<bool> exist;

    m_cacheDataLock.lockForRead();
    exist.store(m_CacheData.contains(tmpKey) && !m_CacheData[tmpKey].value<QPixmap>().isNull());
    m_cacheDataLock.unlock();

    return exist;
}

void IconCacheManager::getPixFromCache(QPair<QString, int> &tmpKey, QPixmap &pix)
{
    m_cacheDataLock.lockForRead();
    pix = m_CacheData[tmpKey].value<QPixmap>();
    m_cacheDataLock.unlock();
}

/**获取小窗口的资源
 * @brief IconCacheManager::loadWindowIcon
 */
void IconCacheManager::loadWindowIcon()
{
    // 加载小窗口图标,写入到缓存
    if (smallWindowLoadState())
        return;

    setIconLoadState(false);

    // 小窗口模式
    const ItemInfoList &itemList = AppsManager::windowedFrameItemInfoList();
    for (int i = 0; i < itemList.size(); i++) {
        const ItemInfo &info = itemList.at(i);
        createPixmap(info, DLauncher::APP_ITEM_ICON_SIZE);
    }

    // 小窗口模式分类图标和文本
    const ItemInfoList &categoryList = AppsManager::windowedCategoryList();
    for (int i = 0; i < categoryList.size(); i++) {
        const ItemInfo &info = categoryList.at(i);
        createPixmap(info, DLauncher::APP_CATEGORY_ICON_SIZE);
    }

    setIconLoadState(true);
    setSmallWindowLoadState(true);
    emit iconLoaded();
}

void IconCacheManager::loadItem(const ItemInfo &info, const QString &operationStr)
{
    if (operationStr == "updated" || operationStr == "deleted")
        removeItemFromCache(info);

    // 小窗口
    createPixmap(info, DLauncher::APP_ITEM_ICON_SIZE);

    // 全屏自由
    int appSize = CalculateUtil::instance()->calculateIconSize(ALL_APPS);
    for (int i = 0; i < ratioList.size(); i++) {
        double ratio = ratioList.at(i);
        int iconWidth = (appSize * ratio);
        createPixmap(info, iconWidth);
    }

    // 全屏分类
    appSize = CalculateUtil::instance()->calculateIconSize(GROUP_BY_CATEGORY);
    for (int i = 0; i < ratioList.size(); i++) {
        double ratio = ratioList.at(i);
        int iconWidth = (appSize * ratio);
        createPixmap(info, iconWidth);
    }
}

void IconCacheManager::loadFullWindowIcon(double ratio)
{
    if (fullFreeLoadState())
        return;

    int appSize = CalculateUtil::instance()->calculateIconSize(ALL_APPS);
    const ItemInfoList itemList = AppsManager::fullscreenItemInfoList();
    for (int i = 0; i < itemList.size(); i++) {
        const ItemInfo &info = itemList.at(i);

        int iconWidth = (appSize * ratio);
        createPixmap(info, iconWidth);
    }
}

/**获取全屏自由模式资源
 * @brief IconCacheManager::loadFullWindowIcon
 */
void IconCacheManager::loadFullWindowIcon()
{
    // 移除小窗口的缓存
    removeSmallWindowCache();

    if (fullFreeLoadState()) {
        setIconLoadState(true);
        return;
    }

    for (int i = 0; i < ratioList.size(); i++) {
        double ratio = ratioList.at(i);
        loadFullWindowIcon(ratio);
    }

    setIconLoadState(true);
    setFullFreeLoadState(true);
    emit iconLoaded();
}

void IconCacheManager::loadCategoryWindowIcon(double ratio)
{
    if (fullCategoryLoadState())
        return;

    // 全屏分类模式,重新计算item的真实大小
    int appSize = CalculateUtil::instance()->calculateIconSize(GROUP_BY_CATEGORY);
    int iconWidth = (appSize * ratio);
    const ItemInfoList itemList = AppsManager::fullscreenItemInfoList();

    for (int i = 0; i < itemList.size(); i++) {
        const ItemInfo &info = itemList.at(i);
        createPixmap(info, iconWidth);
    }
}

void IconCacheManager::preloadCategory()
{
    double ratio = SettingValue("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", "apps-icon-ratio", 0.6).toDouble();
    loadCategoryWindowIcon(ratio);
}

void IconCacheManager::preloadFullFree()
{
    double ratio = SettingValue("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", "apps-icon-ratio", 0.6).toDouble();
    loadFullWindowIcon(ratio);
}

void IconCacheManager::ratioChange(double ratio)
{
    // 预加载－全屏模式另外一种模式当前ratio的资源
    if (!m_launcherInter->fullscreen())
        return;

    switch (m_launcherInter->displaymode()) {
    case ALL_APPS:
        loadCategoryWindowIcon(ratio);
        break;
    case GROUP_BY_CATEGORY:
        loadFullWindowIcon(ratio);
        break;
    }
}

/**获取全屏分类模式资源
 * @brief IconCacheManager::loadCategoryWindowIcon
 */
void IconCacheManager::loadCategoryWindowIcon()
{
    // 移除小窗口的缓存
    removeSmallWindowCache();

    if (fullCategoryLoadState()) {
        setIconLoadState(true);
        return;
    }

    for (int i = 0; i < ratioList.size(); i++) {
        double ratio = ratioList.at(i);
        loadCategoryWindowIcon(ratio);
    }

    setIconLoadState(true);
    setFullCategoryLoadState(true);
    emit iconLoaded();
}

void IconCacheManager::insertCache(const QPair<QString, int> &tmpKey, const QPixmap &pix)
{
    m_cacheDataLock.lockForWrite();

    if (!m_CacheData.contains(tmpKey) || (m_CacheData.contains(tmpKey) && m_CacheData[tmpKey].value<QPixmap>().isNull()))
        m_CacheData[tmpKey] = pix;

    m_cacheDataLock.unlock();
}

void IconCacheManager::removeItemFromCache(const ItemInfo &info)
{
    const int arraySize = 8;
    const int iconArray[arraySize] = { 16, 18, 24, 32, 64, 96, 128, 256 };
    for (int i = 0; i < arraySize; i++) {
        QPair<QString, int> pixKey { cacheKey(info, CacheType::ImageType), iconArray[i] };
        if (existInCache(pixKey)) {
            m_cacheDataLock.lockForWrite();
            m_CacheData.remove(pixKey);
            m_cacheDataLock.unlock();
        }
    }
}

void IconCacheManager::removeSmallWindowCache()
{
    auto removeCache = [](const ItemInfoList &itemList, int pixSize) {
        for (int i = 0; i < itemList.size(); i++) {
            const ItemInfo &itemInfo = itemList.at(i);
            const int iconSize = perfectIconSize(pixSize);
            QPair<QString, int> pixKey { cacheKey(itemInfo, CacheType::ImageType), iconSize };

            if (existInCache(pixKey)) {
                m_cacheDataLock.lockForWrite();
                m_CacheData.remove(pixKey);
                m_cacheDataLock.unlock();
            }
        }
    };


    // 小窗口应用图标
    removeCache(AppsManager::windowedFrameItemInfoList(), DLauncher::APP_ITEM_ICON_SIZE);
    // 小窗口分类图标
    removeCache(AppsManager::AppsManager::windowedCategoryList(), DLauncher::APP_CATEGORY_ICON_SIZE);
    setSmallWindowLoadState(false);
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

void IconCacheManager::setFullFreeLoadState(bool state)
{
    m_fullFreeLoadState.store(state);
}

bool IconCacheManager::fullFreeLoadState()
{
    return m_fullFreeLoadState.load();
}

void IconCacheManager::setFullCategoryLoadState(bool state)
{
    m_fullCategoryLoadState.store(state);
}

bool IconCacheManager::fullCategoryLoadState()
{
    return m_fullCategoryLoadState.load();
}

void IconCacheManager::setSmallWindowLoadState(bool state)
{
    m_smallWindowLoadState = state;
}

bool IconCacheManager::smallWindowLoadState()
{
    return m_smallWindowLoadState.load();
}

void IconCacheManager::updateCanlendarIcon()
{
    static int curDay = QDate::currentDate().day();
    if (curDay != QDate::currentDate().day()) {
        removeItemFromCache(m_calendarInfo);
        createPixmap(m_calendarInfo, m_calendarSize);
    }
}
