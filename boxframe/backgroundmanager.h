/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef BACKGROUNDMANAGER_H
#define BACKGROUNDMANAGER_H

#include <QObject>

#include <com_deepin_wm.h>

class QGSettings;

class BackgroundManager : public QObject
{
    Q_OBJECT
public:
    explicit BackgroundManager(QObject *parent = 0);

    QString currentWorkspaceBackground() const;

    int currentWorkspace() const;
    void setCurrentWorkspace(int currentWorkspace);

signals:
    void currentWorkspaceBackgroundChanged(const QString &background);

private slots:
    void setBackgrounds(QVariant backgroundsVariant);

private:
    int m_currentWorkspace;
    QString m_currentWorkspaceBackground;
    QStringList m_backgrounds;

    com::deepin::wm *m_wmInter;
    QGSettings *m_gsettings;
};

#endif // BACKGROUNDMANAGER_H
