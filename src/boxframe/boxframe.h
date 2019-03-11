/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
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

#ifndef BOXFRAME_H
#define BOXFRAME_H

#include "src/model/backgroundcache.h"

#include <QLabel>
#include <tuple>

#include <com_deepin_wm.h>
#include <com_deepin_daemon_imageblur.h>
#include <com_deepin_daemon_appearance.h>

using WMinter = com::deepin::wm;
using ImageBlurInter = com::deepin::daemon::ImageBlur;
using AppearanceInter = com::deepin::daemon::Appearance;

class QPixmap;
class BoxFrame : public QLabel
{
    Q_OBJECT

public:
    explicit BoxFrame(QWidget* parent = nullptr);
    ~BoxFrame();

    void setBackground(const QString &url);
    inline QPixmap cachePixmap() { return m_cache; }

    virtual void updateGradient() = 0;

protected:
    void updateBackground();
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    const QPixmap backgroundPixmap();
    const std::pair<uint, uint> resolution() const; // std::pair<width, height>
    const QString resolutionToQString(const std::pair<uint, uint> resolution) const;
    const std::pair<uint, uint> resolutionByQString(const QString &resolution) const;
    void updateBackgrounds();
    void onBlurDone(const QString &source, const QString &blur, bool done);
    void saveCurrentPixmap(const QPixmap &pixmap);

private:
    QPixmap m_pixmap;
    QPixmap m_cache;

    int m_currentWorkspace;
    mutable QString m_background;
    BackgroundCache m_currentBackgroundInfo; // std::tuple<md5, resolution, workspace>

    WMinter *m_wmInter;
    ImageBlurInter *m_blurInter;
    AppearanceInter *m_appearanceInter;
};

#endif // BOXFRAME_H
