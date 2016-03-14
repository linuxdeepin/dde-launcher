/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef NAVIGATIONBUTTONFRAME_H
#define NAVIGATIONBUTTONFRAME_H

#include <QStringList>
#include <QFrame>

class BaseCheckedButton;
class QButtonGroup;

const QStringList CategoryKeys {
    "Internet",
    "Chat",
    "Music",
    "Video",
    "Graphics",
    "Game",
    "Office",
    "Reading",
    "Development",
    "System",
    "Others"
};

class NavigationButtonFrame : public QFrame
{
    Q_OBJECT
public:
    explicit NavigationButtonFrame(int mode, QWidget *parent = 0);
    ~NavigationButtonFrame();

    void initConnect();
    void initByMode(int mode);

signals:
    void currentIndexChanged(int index);
public slots:
    void hideButtons(const QList<qlonglong>& keys);
    void handleButtonClicked(int id);
    void checkButtonByKey(qlonglong key);
    void setCurrentIndex(int currentIndex);
    void addTextShadow();

    void checkFirstButton();

private:
    QButtonGroup* m_buttonGroup;
    int m_currentIndex = 0;
};

#endif // NAVIGATIONBUTTONFRAME_H
