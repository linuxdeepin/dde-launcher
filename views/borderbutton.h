/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef BORDERBUTTON_H
#define BORDERBUTTON_H

#include <QPushButton>
#include <QPoint>
class QMouseEvent;
class QWidget;

class BorderButton : public QPushButton
{
    Q_OBJECT
public:
    explicit BorderButton(QWidget *parent = 0);
    ~BorderButton();

    bool isHighlight() const;
    void updateStyle();
    void drawBorder(int borderWidth, int radius, QColor borderColor, QColor brushColor);

signals:
    void rightClicked(QPoint pos);
    void graphicsEffectOn();
    void graphicsEffectOff();

public slots:
    void setHighlight(bool isHightlight);
    void toggleHighlight();
    void startDrag(QMouseEvent* event);
    void setCuted(bool isCuted);
    void addTextShadow();

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void paintEvent(QPaintEvent* event);

private:
    bool m_checked = false;
    bool m_isHighlight = false;
    bool m_isCuted = false;
    bool m_isRightPressed = false;
};

#endif // BORDERBUTTON_H
