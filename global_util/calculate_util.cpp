#include "calculate_util.h"

#include <QDebug>

CalculateUtil *CalculateUtil::INSTANCE = nullptr;

//void CalculateUtil::calAppSize(QSize desktopSize) {
//    qDebug() << "desktopSize:" << desktopSize;
//    int column = 7;

//    int minWidth =qMin(desktopSize.width(), desktopSize.height());
//    if (minWidth >= 768) {
//        column = 7;
//    } else if (minWidth < 768 && minWidth >= 600) {
//        column = 5;
//    } else {
//        column = 4;
//    }

//    app_item_width = (desktopSize.width() - app_item_minspacing*column)/column;
//    app_item_width = qMin(app_item_width, 130);

//    app_item_spacing = (desktopSize.width() - app_item_width*column)/column - 8;
//    app_item_spacing = qMax(app_item_spacing, 10);

//    app_item_height = app_item_width;

//    if (app_item_width > 2*64) {
//        app_icon_size = 64;
//    } else {
//        app_icon_size = 48;
//    }

//    if (app_item_width >= 130) {
//        app_item_font_size = 12;
//    } else if (app_item_width < 130 && app_item_width >=100) {
//        app_item_font_size = 10;
//    } else if (app_item_width > 80 && app_item_width < 100) {
//        app_item_font_size = 8;
//    } else {
//        app_item_font_size = 4;
//    }

//    qDebug() << "app_item_width: " << app_item_width << "column:" << column;
//    qDebug() << "app_spacing:" << app_item_spacing;
//    qDebug() << "app_item_font_size:" << app_item_font_size;


//}

CalculateUtil *CalculateUtil::instance(QObject *parent)
{
    if (!INSTANCE)
        INSTANCE = new CalculateUtil(parent);

    return INSTANCE;
}

CalculateUtil::CalculateUtil(QObject *parent) : QObject(parent)
{

}
