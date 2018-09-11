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

#include <com_deepin_wm.h>
#include <com_deepin_daemon_accounts_user.h>
#include <com_deepin_daemon_imageblur.h>

using ImageBlurInter = com::deepin::daemon::ImageBlur;
using User = com::deepin::daemon::accounts::User;

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
    void updateBackgrounds();
    void onDesktopWallpapersChanged(const QStringList &files);
    void onBlurDone(const QString &source, const QString &blur, bool done);

private:
    int m_currentWorkspace;
    QStringList m_backgrounds;

    com::deepin::wm *m_wmInter;
    ImageBlurInter *m_blurInter;
    User *m_currentUser;
};

#endif // BACKGROUNDMANAGER_H
