/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef LAUNCHERFRAME_H
#define LAUNCHERFRAME_H

#include <QFrame>
#include <QMouseEvent>
#include <QInputEvent>
#include <dbusinterface/dbustype.h>

class CategoryFrame;
class AppTableWidget;
class DisplayModeFrame;
class QStackedLayout;
class QButtonGroup;
class QPushButton;
class QCloseEvent;
class SearchLineEdit;
class BackgroundLabel;
class GradientLabel;

class LauncherFrame : public QFrame
{
    Q_OBJECT

public:
    explicit LauncherFrame(QWidget *parent = 0);
    ~LauncherFrame();
    static int AppItemTopSpacing;
    static int AppItemIconNameSpacing;
    static int AppItemMargin;
    static int IconSize;
    static int TextHeight;
    static int GridSpacing;
    static int GridSize;
    static int BorderSize;
    static int BorderWidth;
    static int BorderHeight;
    static int ColumnCount;
    static int RowCount;

    void initUI();
    void initConnect();
    void computerGrid(int minimumLeftMargin, int minimumTopMargin, int miniSpacing ,int itemWidth);
    int currentMode();

signals:
    void Closed();
    void Shown();

public slots:
    void showSortedMode(int mode);
    void showCategoryMode(int mode);
    void toggleDisableNavgationBar(bool flag);
    void showAppTableWidgetByMode(int mode);
    void showNavigationBarByMode();
    void handleMouseReleased();
    void showAppTableWidget();
    void showCategoryTable();
    void showSearchResult(const QList<ItemInfo>& infoList);
    void Exit();
    void Hide();
    void Show();
    void ShowByMode(qlonglong mode);
    void Toggle();
    void handleToggle();
    void handleSearch(const QString& text);
    void search();
    void clearSearchEdit();
    void backNormalView();
    void handleAppOpened(const QString& appUrl);
    void uninstallUpdateTable(QString appKey);
    void setRightclicked(bool flag);
    void handleScreenGeometryChanged();
    void showGradients() const;
    void updateGradients(QPixmap) const;
    void handleActiveWindowChanged(uint windowId);
    void handleContentScrolled(int value);

    void setDragging(bool flag);

protected:
    void mouseReleaseEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void closeEvent(QCloseEvent* event);
//    void changeEvent(QEvent* event);
    void inputMethodEvent(QInputMethodEvent *event);
    void wheelEvent(QWheelEvent * event);
    bool eventFilter(QObject *obj, QEvent *event);


private:
    int m_leftMargin;
    int m_rightMargin;
    int m_topMargin;
    int m_bottomMargin;
    int m_spacing;
    int m_column;
    int m_row;
    int m_itemWidth;
    int m_gridWidth;
    int m_gridHeight;
    bool m_rightclicked = false;
    bool m_isDraging = false;
    //record the initial times of categoryframe
    int count=0;
    QPoint m_primaryPos;
    QString m_searchText;
    QStackedLayout* m_layout;
    DisplayModeFrame* m_displayModeFrame;
    CategoryFrame* m_categoryFrame;
    AppTableWidget* m_appTableWidget;
    QPushButton* m_clearCheckedButton;
    SearchLineEdit* m_searchLineEdit;
    BackgroundLabel* m_backgroundLabel;
    GradientLabel * m_topGradient;
    GradientLabel * m_bottomGradient;
    int m_currentIndex = 0;
    QTimer* m_searchTimer;
};

#endif // LAUNCHERFRAME_H
