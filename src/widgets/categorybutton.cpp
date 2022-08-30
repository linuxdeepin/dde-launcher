// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "categorybutton.h"
#include "constants.h"
#include "util.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QApplication>
#include <QSvgRenderer>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>

CategoryButton::CategoryButton(const AppsListModel::AppCategory category, QWidget *parent) :
    QAbstractButton(parent)
    , m_category(category)
    , m_fScale(1)
{
    setAccessibleName("Btn-Category");
    setObjectName("CategoryButton");
    setCheckable(true);
    setAutoExclusive(true);
    setFocusPolicy(Qt::NoFocus);
    setFixedSize(DLauncher::NAVIGATION_BUTTON_SIZE, DLauncher::NAVIGATION_BUTTON_SIZE);
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
    m_icon = loadSvg(QString(":/icons/skin/icons/category_%1%2.svg").arg(m_iconName, m_systemTheme), qRound(DLauncher::NAVIGATION_ICON_SIZE * devicePixelRatioF()));
    m_icon.setDevicePixelRatio(qApp->devicePixelRatio());
}

void CategoryButton::updateState(const CategoryButton::State state)
{
    m_state = state;
    update();
}

void CategoryButton::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    updateState(Hover);
}

void CategoryButton::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    if (isChecked())
        updateState(Checked);
    else
        updateState(Normal);
}

void CategoryButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        updateState(Press);
    }
}

void CategoryButton::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);
    if (isChecked())
        updateState(Checked);
    else
        updateState(Normal);
    emit clicked(true);
}

void CategoryButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);

    switch (m_state) {
    case Hover:
        p.setBrush(QBrush(QColor(255, 255, 255, 0.2 * 255)));
        break;
    case Checked:
        p.setBrush(QBrush(QColor(255, 255, 255, 0.3 * 255)));
        break;
    case Press:
        p.setBrush(QBrush(QColor(255, 255, 255, 0.1 * 255)));
        break;
    default:
        break;
    }

    p.drawEllipse(rect());
    const auto ratio = devicePixelRatioF();
    const QRectF &rf = QRectF(rect());
    const QRectF &rfp = QRectF(m_icon.rect());
    p.drawPixmap(rf.center() - rfp.center() / ratio, m_icon);
}
