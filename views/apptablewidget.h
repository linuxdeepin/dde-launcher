#ifndef APPTABLEWIDGET_H
#define APPTABLEWIDGET_H

#include "basetablewidget.h"
#include "fwd.h"
#include "dbusinterface/dbustype.h"

class QWheelEvent;

class AppTableWidget : public BaseTableWidget
{
    Q_OBJECT
public:
    explicit AppTableWidget(QWidget *parent = 0);
    ~AppTableWidget();

    void initConnect();
    void setGridParameter(int column, int girdWidth, int itemWidth);

signals:

public slots:
    void addItem(ItemInfo itemInfo, int index);
    void addItems(const QList<ItemInfo>& itemInfos);

    void setItemInfosMap(const QMap<QString, ItemInfo>& itemInfos);

    void showbyName();
    void showByInstalledTime();
    void showByFrequency();
    void showBySortedMode(int mode);

protected:
    void wheelEvent(QWheelEvent* event);

private:
    int m_column;
    int m_gridWidth;
    int m_itemWidth;
    QMap<QString, ItemInfo> m_itemInfosMap;
};

#endif // APPTABLEWIDGET_H
