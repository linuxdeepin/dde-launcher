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

class QPixmap;
class BackgroundManager;
class QScreen;

class BoxFrame : public QLabel
{
    Q_OBJECT

public:
    explicit BoxFrame(QWidget* parent = nullptr);
    ~BoxFrame() Q_DECL_OVERRIDE;

    void setBackground(const QString &url);
    void setBlurBackground(const QString &url);
    inline QPixmap cachePixmap() { return m_cache; }

signals:
    void backgroundImageChanged(const QPixmap & img);

protected:
    void updateBlurBackground();
    void updateBackground();
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    const QPixmap backgroundPixmap();
    virtual const QScreen * currentScreen();
private:
    QString m_lastUrl;
    QString m_lastBlurUrl;
    QPixmap m_pixmap;
    QPixmap m_cache;
    BackgroundManager *m_bgManager;
};

#endif // BOXFRAME_H
