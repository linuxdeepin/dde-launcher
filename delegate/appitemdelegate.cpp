
#include "appitemdelegate.h"
#include "global_util/constants.h"
#include "global_util/calculate_util.h"
#include "model/appslistmodel.h"
#include "dbusinterface/dbusvariant/iteminfo.h"

#include <QDebug>
#include <QPixmap>
#include <QVariant>

QModelIndex AppItemDelegate::CurrentIndex = QModelIndex();

AppItemDelegate::AppItemDelegate(QObject *parent) :
    QAbstractItemDelegate(parent),
    m_calcUtil(CalculateUtil::instance(this)),
    m_blueDotPixmap(":/skin/images/new_install_indicator.png"),
    m_autoStartPixmap(":/skin/images/emblem-autostart.png")
{

}

void AppItemDelegate::setCurrentIndex(const QModelIndex &index)
{
    if (CurrentIndex == index)
        return;

    const QModelIndex previousIndex = CurrentIndex;
    CurrentIndex = index;

    emit currentChanged(previousIndex, CurrentIndex);
}

void AppItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data(AppsListModel::AppItemIsDragingRole).value<bool>() && !(option.features & QStyleOptionViewItem::HasDisplay))
        return;

    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter->setPen(Qt::white);
    painter->setBrush(QBrush(Qt::transparent));

    const int leftMargin = 2, radius = 10;
    const int fontPixelSize = index.data(AppsListModel::AppFontSizeRole).value<int>();
    const bool drawBlueDot = index.data(AppsListModel::AppNewInstallRole).toBool();
    const ItemInfo itemInfo = index.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo>();
    const QRect boundingRect = itemBoundingRect(option.rect);
    const QSize iconSize = index.data(AppsListModel::AppIconSizeRole).toSize();

    // draw focus background
   if (CurrentIndex == index && !(option.features & QStyleOptionViewItem::HasDisplay))
    {
        const QColor borderColor(245, 245, 245, 0.01 * 255);
        const QColor brushColor(0, 0, 0, 105);

        QPen pen;
        pen.setColor(borderColor);
        pen.setWidth(2);
        QPainterPath border;
        border.addRoundedRect(boundingRect.marginsRemoved(QMargins(leftMargin/2, leftMargin/2, leftMargin*2/3, leftMargin*2/3)),
                              radius, radius);
        painter->strokePath(border, pen);
        painter->setPen(Qt::transparent);
        painter->setBrush(brushColor);
        painter->drawRoundedRect(boundingRect.marginsRemoved(QMargins(leftMargin, leftMargin, leftMargin*4/3, leftMargin*4/3)),
                                 radius, radius);
    }

    // draw app icon
    const int iconLeftMargins = (boundingRect.width() - iconSize.width()) / 2;
    const int iconTopMargin = qMin(10, int(boundingRect.height() * 0.1));
    const QRect iconRect = QRect(boundingRect.topLeft() + QPoint(iconLeftMargins, iconTopMargin), iconSize);
    painter->drawPixmap(iconRect, index.data(AppsListModel::AppIconRole).value<QPixmap>());

    // draw icon if app is auto startup
    const QPoint autoStartIconPos = iconRect.bottomLeft() - QPoint(0, m_autoStartPixmap.height());
    if (index.data(AppsListModel::AppAutoStartRole).toBool())
        painter->drawPixmap(autoStartIconPos, m_autoStartPixmap);

    // draw app name
    QTextOption appNameOption;
    appNameOption.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    appNameOption.setWrapMode(QTextOption::WordWrap);
    QFont appNamefont(painter->font());
    appNamefont.setPixelSize(fontPixelSize);

    const QFontMetrics fm(appNamefont);
    const QRectF appNameRect = itemTextRect(boundingRect, iconRect, drawBlueDot);
//    const QRectF appNameBoundingRect = fm.boundingRect(appNameRect.toRect(), appNameOption.alignment() | wrapFlag, itemInfo.m_name);
    const QString appText = holdTextInRect(fm, itemInfo.m_name, appNameRect.toRect());
//    const QString appText = appNameBoundingRect.width() > appNameRect.width() || appNameBoundingRect.height() > appNameRect.height()
//                                ? fm.elidedText(itemInfo.m_name, Qt::ElideRight, appNameRect.width(), appNameOption.alignment() | wrapFlag)
//                                : itemInfo.m_name;

    painter->setFont(appNamefont);
    painter->setBrush(QBrush(Qt::transparent));
    painter->setPen(QColor(0, 0, 0, 80));
    painter->drawText(appNameRect.adjusted(0.8, 1, 0.8, 1), appText, appNameOption);
    painter->drawText(appNameRect.adjusted(-0.8, 1, -0.8, 1), appText, appNameOption);
    painter->setPen(Qt::white);
    painter->drawText(appNameRect, appText, appNameOption);

    // draw blue dot if needed
    if (drawBlueDot)
    {
        const int marginRight = 2;
        const QRectF textRect = fm.boundingRect(appNameRect.toRect(), Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap, appText);

        const QPointF blueDotPos = textRect.topLeft() + QPoint(-m_blueDotPixmap.width() - marginRight, (fm.height() - m_blueDotPixmap.height()) / 2);
        painter->drawPixmap(blueDotPos, m_blueDotPixmap);
    }
}

QSize AppItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    return index.data(AppsListModel::ItemSizeHintRole).toSize();
}

///
/// \brief calculate item background bounding rect, this rect contains all item like icon, text, blue dot, ...
/// \param itemRect the real item rect
/// \return item bounding rect
///
const QRect AppItemDelegate::itemBoundingRect(const QRect &itemRect) const
{
    const int w = itemRect.width();
    const int h = itemRect.height();
    const int sub = qAbs((w - h) / 2);

    if (w == h)
        return itemRect;
    else if (w > h)
        return itemRect - QMargins(sub, 0, sub, 0);
    else
        return itemRect - QMargins(0, sub, 0, sub);
}

///
/// \brief calculate app item text area rect
/// \param boundingRect item bounding rect
/// \param iconRect item icon rect
/// \param extraWidthMargin remove extra margin if need draw blue dot
/// \return app name text bounding rect
///
const QRect AppItemDelegate::itemTextRect(const QRect &boundingRect, const QRect &iconRect, const bool extraWidthMargin) const
{
    const int widthMargin = extraWidthMargin ? 16 : 2;
    const int heightMargin = 2;

    QRect result = boundingRect;

    result.setTop(iconRect.bottom() + 15);

    return result.marginsRemoved(QMargins(widthMargin, heightMargin, widthMargin, heightMargin));
}

const QString AppItemDelegate::holdTextInRect(const QFontMetrics &fm, const QString &text, const QRect &rect) const
{
    const int textFlag = Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap;

    if (rect.contains(fm.boundingRect(rect, textFlag, text)))
        return text;

    QString str(text + "...");

    while (true)
    {
        if (str.size() < 4)
            break;

        QRect boundingRect = fm.boundingRect(rect, textFlag, str);
        if (rect.contains(boundingRect))
            break;

        str.remove(str.size() - 4, 1);
    }

    return str;
}
