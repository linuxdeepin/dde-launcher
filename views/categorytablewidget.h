#ifndef SECTIONTABLEWIDGET_H
#define SECTIONTABLEWIDGET_H

#include "basetablewidget.h"
#include "dbusinterface/dbustype.h"
class QWheelEvent;
class CategoryItem;
class AppItem;
class QPropertyAnimation;

class CategoryTableWidget : public BaseTableWidget
{
    Q_OBJECT
public:
    explicit CategoryTableWidget(QWidget *parent = 0);
    ~CategoryTableWidget();
    void initConnect();

signals:
    void verticalPositionChanged(int pos);

public slots:
    void show();
    void setGridParameter(int column, int girdWidth, int gridHeight, int itemWidth);

    void setCategoryInfoList(const CategoryInfoList& categoryInfoList);
    void showAppItems();

    void addCategoryItem(int row,  qlonglong key);
    void addItems(int row, qlonglong categoryKey, QStringList appKeys);
    void addItems(const CategoryInfoList& categoryInfoList);

    void handleScrollBarValueChanged(int value);
    void scrollToCategory(qlonglong key);

    void openCheckedApp();
    void showAutoStartLabel(QString appKey);
    void hideAutoStartLabel(QString appKey);

    void removeItem(QString appKey);

    void handleCurrentAppItemChanged(QString appKey);

    void handleWheelEvent(QWheelEvent * event);

protected:
    void showEvent(QShowEvent * event);
    void wheelEvent(QWheelEvent * event);
private:
    int m_column;
    int m_gridWidth;
    int m_gridHeight;
    int m_itemWidth;

    QPropertyAnimation* m_scrollAnimation;
    QStringList m_keyused;
};

#endif // SECTIONTABLEWIDGET_H
