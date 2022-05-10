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
static QList<double> ratioList = { 0.2, 0.3, 0.4, 0.5, 0.6 };
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
{
    setIconLoadState(false);
}

void IconCacheManager::createPixmap(const ItemInfo &itemInfo, int size)
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
    const ItemInfoList &itemList = AppsManager::windowedFrameItemInfoList();
    for (int i = 0; i < itemList.size(); i++) {
        const ItemInfo &info = itemList.at(i);
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
    // 小窗口模式分类图标
    const ItemInfoList &categoryList = AppsManager::windowedCategoryList();
    for (int i = 0; i < categoryList.size(); i++) {
        const ItemInfo &info = categoryList.at(i);
        createPixmap(info, DLauncher::APP_CATEGORY_ICON_SIZE);
    }

    // 小窗口模式卸载,拖拽图标
    const ItemInfoList &itemList = AppsManager::windowedFrameItemInfoList();
    for (int i = 0; i < itemList.size(); i++) {
        const ItemInfo &info = itemList.at(i);
        createPixmap(info, DLauncher::APP_DLG_ICON_SIZE);
        createPixmap(info, DLauncher::APP_DRAG_ICON_SIZE);
    }
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

void IconCacheManager::loadCurRatioIcon(int mode)
{
    const ItemInfoList &itemList = AppsManager::fullscreenItemInfoList();
    int appSize = CalculateUtil::instance()->calculateIconSize(mode);

    for (int i = 0; i < itemList.size(); i++) {
        const ItemInfo &info = itemList.at(i);
        createPixmap(info, appSize * getCurRatio());
    }

    setIconLoadState(true);
    emit iconLoaded();
}

void IconCacheManager::loadOtherRatioIcon(int mode)
{
    int appSize = CalculateUtil::instance()->calculateIconSize(mode);
    const ItemInfoList &itemList = AppsManager::fullscreenItemInfoList();
    for (int i = 0; i < ratioList.size(); i++) {
        double ratio = ratioList.at(i);
        if (qFuzzyCompare(getCurRatio(), ratio))
            continue;

        for (int j = 0; j < itemList.size(); j++) {
            const ItemInfo &info = itemList.at(j);
            createPixmap(info, appSize * ratio);
        }
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

void IconCacheManager::removeItemFromCache(const ItemInfo &info)
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
    auto removeCache = [](const ItemInfoList &itemList, int pixSize) {
        for (int i = 0; i < itemList.size(); i++) {
            const ItemInfo &itemInfo = itemList.at(i);
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
    removeCache(AppsManager::windowedFrameItemInfoList(), DLauncher::APP_ITEM_ICON_SIZE);
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
    static int curDay = QDate::currentDate().day();
    if (curDay != QDate::currentDate().day()) {
        removeItemFromCache(m_calendarInfo);

        for (int i = 0; i < sizeList.size(); i++)
            createPixmap(m_calendarInfo, sizeList.at(i));
    }
}
