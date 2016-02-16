/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef BASETABLEWIDGET_H
#define BASETABLEWIDGET_H

#include <QTableWidget>
class QMouseEvent;
class QHideEvent;

class BaseTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit BaseTableWidget(QWidget *parent = 0);
    ~BaseTableWidget();

    void firstHighlight();
    void hightlightItem(int row, int column);

    void handleKeyUpPressed();
    void handleKeyDownPressed();
    void handleKeyLeftPressed();
    void handleKeyRightPressed();
    int getHighLightRow() const;
    int getHighLightColumn() const;

signals:
    void currentAppItemChanged(QString appKey);

public slots:
    void clearHighlight();
    void setItemUnChecked();
    void handleDirectionKeyPressed(Qt::Key key);
    void setHighLight(int hRow, int hColumn);

protected:
    void mouseReleaseEvent(QMouseEvent* event);
    void hideEvent(QHideEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);

private:
    int m_highlightRow = -1;
    int m_highlightColumn = -1;

};

#endif // BASETABLEWIDGET_H
