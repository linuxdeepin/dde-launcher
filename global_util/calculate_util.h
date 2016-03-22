#ifndef CALCULATE_UTIL_H
#define CALCULATE_UTIL_H

#include <QObject>
#include <QSize>

#include <QtCore>

class CalculateUtil:public QObject{
    Q_OBJECT
public:
    CalculateUtil(QSize size, QObject* parent = 0);
    ~CalculateUtil();

    int app_item_width = 130;
    int app_item_height = 130;
    int app_item_spacing = 10;
    int app_item_minspacing = 20;
    int app_icon_size = 64;
    int app_item_font_size = 12;

public slots:
    void calAppSize(QSize desktopSize);
};

#endif // CALCULATE_UTIL_H
