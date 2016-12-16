/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "backgroundmanager.h"

#include <QGSettings>

using namespace com::deepin;

static const QString DefaultWallpaper = "/usr/share/backgrounds/default_background.jpg";

BackgroundManager::BackgroundManager(QObject *parent)
    : QObject(parent),
      m_currentWorkspace(0),
      m_wmInter(new wm("com.deepin.wm", "/com/deepin/wm", QDBusConnection::sessionBus(), this)),
      m_gsettings(new QGSettings("com.deepin.dde.appearance", "", this))
{
    auto updateBackgrounds = [this] {
        setBackgrounds(m_gsettings->get("background-uris"));
    };
    auto updateWorkspace = [this] (int, int to) {
        setCurrentWorkspace(to);
    };

    connect(m_gsettings, &QGSettings::changed, updateBackgrounds);
    connect(m_wmInter, &__wm::WorkspaceSwitched, updateWorkspace);

    updateBackgrounds();
    updateWorkspace(0, 0);
}

QString BackgroundManager::currentWorkspaceBackground() const
{
    if (m_backgrounds.length() >= m_currentWorkspace + 1) {
        return m_backgrounds.at(m_currentWorkspace);
    }

    return DefaultWallpaper;
}

void BackgroundManager::setBackgrounds(QVariant backgrounsVariant)
{
    m_backgrounds = backgrounsVariant.toStringList();

    QString background = currentWorkspaceBackground();
    if (background != m_currentWorkspaceBackground) {
        m_currentWorkspaceBackground = background;

        emit currentWorkspaceBackgroundChanged(background);
    }
}

int BackgroundManager::currentWorkspace() const
{
    return m_currentWorkspace;
}

void BackgroundManager::setCurrentWorkspace(int currentWorkspace)
{
    if (m_currentWorkspace != currentWorkspace) {
        m_currentWorkspace = currentWorkspace;
        QString background =  currentWorkspaceBackground();

        if (m_currentWorkspaceBackground != background) {
            m_currentWorkspaceBackground = background;

            emit currentWorkspaceBackgroundChanged(background);
        }
    }
}
