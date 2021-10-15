/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BACKGROUNDMANAGER_H
#define BACKGROUNDMANAGER_H

#include <QObject>
#include <QDesktopWidget>
#include <QScreen>
#include <DSingleton>

#include <com_deepin_wm.h>
#include <com_deepin_daemon_imageeffect.h>
#include <com_deepin_daemon_imageblur.h>
#include <com_deepin_daemon_appearance.h>
#include <com_deepin_daemon_display.h>

#define CUSTOM_MODE     0
#define MERGE_MODE      1
#define EXTEND_MODE     2
#define SINGLE_MODE     3

using ImageEffeblur = com::deepin::daemon::ImageBlur;
using ImageEffectInter = com::deepin::daemon::ImageEffect;
using AppearanceInter = com::deepin::daemon::Appearance;
using DisplayInter = com::deepin::daemon::Display;

class DisplayHelper : public QObject, public Dtk::Core::DSingleton<DisplayHelper>
{
    Q_OBJECT
    friend class Dtk::Core::DSingleton<DisplayHelper>;

public:
    inline int displayMode() const { return m_displayMode;}

private:
    explicit DisplayHelper(QObject *parent = nullptr);

private slots:
    void updateDisplayMode();

private:
    QPointer<DisplayInter> m_displayInter;
    int m_displayMode;
};


class BackgroundManager : public QObject
{
    Q_OBJECT
public:
    explicit BackgroundManager(QObject *parent = nullptr);

    int dispalyMode() const { return m_displayMode; }

private:
    void getImageDataFromDbus(const QString &filePath);

signals:
    void currentWorkspaceBackgroundChanged(const QString &background);
    void currentWorkspaceBlurBackgroundChanged(const QString &background);
    void blurImageAcquired();

public slots:
    void updateBlurBackgrounds();
    void onAppearanceChanged(const QString & type, const QString &str);
    void onDisplayModeChanged(uchar  value);
    void onPrimaryChanged(const QString & value);
    void onGetBlurImageFromDbus(const QString &file, const QString &blurFile, bool status);

private:
    int m_currentWorkspace;
    mutable QString m_blurBackground;
    mutable QString m_background;

    QPointer<com::deepin::wm> m_wmInter;
    QPointer<ImageEffectInter> m_imageEffectInter;
    QPointer<ImageEffeblur> m_imageblur;
    QPointer<AppearanceInter> m_appearanceInter;
    QPointer<DisplayInter> m_displayInter;
    int m_displayMode;
    QString m_fileName;
};

#endif // BACKGROUNDMANAGER_H
