#ifndef CALCULATE_UTIL_H
#define CALCULATE_UTIL_H

#include <QObject>
#include <QSize>

#include <QtCore>

class CalculateUtil : public QObject
{
    Q_OBJECT

public:
    static CalculateUtil *instance(QObject *parent = nullptr);

    inline int appItemFontSize() const {return m_appItemFontSize;}
    inline int appIconSize() const {return m_appIconSize;}
    inline int appItemSpacing() const {return m_appItemSpacing;}
    inline QSize appItemSize() const {return QSize(m_appItemWidth, m_appItemHeight);}

#ifdef QT_DEBUG
    inline void increaseIconSize() {m_appIconSize += 16;}
    inline void decreaseIconSize() {m_appIconSize -= 16;}
    inline void increaseItemSize() {m_appItemWidth += 16; m_appItemHeight += 16;}
    inline void decreaseItemSize() {m_appItemWidth -= 16; m_appItemHeight -= 16;}
#endif

private:
    explicit CalculateUtil(QObject *parent);

private:
    static CalculateUtil *INSTANCE;

    int m_appItemFontSize = 12;
    int m_appIconSize = 64;
    int m_appItemSpacing = 10;
    int m_appItemWidth = 130;
    int m_appItemHeight = 130;

//    int app_item_width = 130;
//    int app_item_height = 130;
//    int app_item_spacing = 10;
//    int app_item_minspacing = 20;
//    int app_icon_size = 64;
//    int app_item_font_size = 12;
};

#endif // CALCULATE_UTIL_H
