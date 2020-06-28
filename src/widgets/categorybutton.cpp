/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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

#include "categorybutton.h"
#include "src/global_util/constants.h"
#include "src/global_util/util.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QSvgRenderer>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>

CategoryButton::CategoryButton(const AppsListModel::AppCategory category, QWidget *parent) :
    QAbstractButton(parent)
    , m_calcUtil(CalculateUtil::instance())
    , m_category(category)
    , m_fScale(1)
{
    setAccessibleName("Btn-Category");
    setObjectName("CategoryButton");
    setCheckable(true);
    setAutoExclusive(true);
    setFocusPolicy(Qt::NoFocus);
    setFixedSize(QSize(DLauncher::NAVIGATION_BUTTON_SIZE, DLauncher::NAVIGATION_BUTTON_SIZE));
    setInfoByCategory();
    connect(this, &CategoryButton::toggled, this, &CategoryButton::setChecked);
}

AppsListModel::AppCategory CategoryButton::category() const
{
    return m_category;
}

void CategoryButton::setChecked(bool isChecked)
{
    if (isChecked)
        updateState(Checked);
    else
        updateState(Normal);

    QAbstractButton::setChecked(isChecked);
}

void CategoryButton::setScreenScale(double fScale)
{
    m_fScale = fScale;
    setInfoByCategory();
}

void CategoryButton::setInfoByCategory()
{
    switch (m_category) {
    case AppsListModel::Internet:       setAccessibleName("internet");
        m_iconName = "network";                    break;
    case AppsListModel::Chat:           setAccessibleName("chat");
        m_iconName = "chat";                        break;
    case AppsListModel::Music:          setAccessibleName("music");
        m_iconName = "music";                       break;
    case AppsListModel::Video:          setAccessibleName("multimedia");
        m_iconName = "video";                  break;
    case AppsListModel::Graphics:       setAccessibleName("graphics");
        m_iconName = "graphic";                    break;
    case AppsListModel::Game:           setAccessibleName("game");
        m_iconName = "game";                        break;
    case AppsListModel::Office:         setAccessibleName("office");
        m_iconName = "office";                      break;
    case AppsListModel::Reading:        setAccessibleName("reading");
        m_iconName = "reading";                     break;
    case AppsListModel::Development:    setAccessibleName("development");
        m_iconName = "develop";                 break;
    case AppsListModel::System:         setAccessibleName("system");
        m_iconName = "system";                      break;
    default:                            setAccessibleName("others");
        m_iconName = "others";                      break;
    }

//    const auto ratio = devicePixelRatioF();
    m_systemTheme = "_dark";
    m_icon = loadSvg(QString(":/icons/skin/icons/category_%1%2.svg").arg(m_iconName, m_systemTheme), DLauncher::NAVIGATION_ICON_SIZE*qApp->devicePixelRatio());
    m_icon.setDevicePixelRatio(qApp->devicePixelRatio());
}

void CategoryButton::updateState(const CategoryButton::State state)
{
    m_state = state;
    update();
}

void CategoryButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    if (m_state == Checked) {
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(QColor(255, 255, 255, 0.3 * 255)));
        p.drawEllipse(rect());
    }

    const auto ratio = devicePixelRatioF();
    const QRectF &rf = QRectF(rect());
    const QRectF &rfp = QRectF(m_icon.rect());
    p.drawPixmap(rf.center() - rfp.center() / ratio, m_icon);
}
