/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef BOXFRAME_H
#define BOXFRAME_H

#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QEvent>

#include <QFrame>
#include <QLabel>
#include <QFileSystemWatcher>

class BoxFrame : public QFrame
{
    Q_OBJECT

public:
    explicit BoxFrame(QWidget* parent = 0);
    explicit BoxFrame(const QString &url, QWidget *parent = 0);
    ~BoxFrame();

    void setBackground(const QString &url, bool force = false);
    QPixmap getBackground();

signals:
    void backgroundChanged();

private slots:
    void resetBackground();

private:
    QString m_lastUrl;
    QPixmap m_pixmap;
    QPixmap m_cache;
    QFileSystemWatcher m_blurredImageWatcher;
};

#endif // BOXFRAME_H
