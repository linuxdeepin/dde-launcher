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

#include <QLabel>
#include <QPixmapCache>

class QPixmap;
class BackgroundManager;
class QScreen;

class BoxFrame : public QLabel
{
    Q_OBJECT

public:
    explicit BoxFrame(QWidget* parent = nullptr);

    void setBackground(const QString &url);
    void setBlurBackground(const QString &url);

signals:
    void backgroundImageChanged(const QPixmap & img);

protected:
    void scaledBackground();
    void scaledBlurBackground();
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void moveEvent(QMoveEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    virtual const QScreen * currentScreen();

private:
    QString m_lastUrl;
    QString m_lastBlurUrl;
    QPixmap m_pixmap;
    QString m_defaultBg;
    BackgroundManager *m_bgManager;
    bool m_useSolidBackground;
};

#endif // BOXFRAME_H
