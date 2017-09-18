/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
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
#include "global_util/constants.h"
#include "global_util/util.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QApplication>

CategoryButton::CategoryButton(const AppsListModel::AppCategory category, QWidget *parent) :
    QAbstractButton(parent),
    m_calcUtil(CalculateUtil::instance()),
    m_category(category),
    m_iconLabel(new QLabel),
    m_textLabel(new QLabel),
    m_opacityAnimation(new QPropertyAnimation(this, "titleOpacity"))
{
    const auto ratio = qApp->devicePixelRatio();

    setObjectName("CategoryButton");
    m_iconLabel->setFixedSize(22 * ratio, 22 * ratio);
    m_textLabel->setFixedHeight(22 * ratio);
    m_textLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_iconLabel);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(m_textLabel);
    mainLayout->addStretch(3);

    setLayout(mainLayout);
    setCheckable(true);
    setAutoExclusive(true);
    setFocusPolicy(Qt::NoFocus);
    setFixedHeight(DLauncher::NAVIGATION_ICON_HEIGHT);
    setInfoByCategory();
    updateState(Normal);
    addTextShadow();

    m_opacityAnimation->setDuration(300);

    connect(this, &CategoryButton::toggled, this, &CategoryButton::setChecked);
    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, &CategoryButton::relayout);
    connect(m_opacityAnimation, &QPropertyAnimation::finished, [this] {
        m_textLabel->setVisible(m_titleOpacity != 0);
    });
}

AppsListModel::AppCategory CategoryButton::category() const
{
    return m_category;
}

void CategoryButton::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
}

void CategoryButton::enterEvent(QEvent *e)
{
    if (!isChecked())
        updateState(Hover);

    QAbstractButton::enterEvent(e);
}

void CategoryButton::leaveEvent(QEvent *e)
{
    if (!isChecked())
        updateState(Normal);

    QAbstractButton::leaveEvent(e);
}

void CategoryButton::setChecked(bool isChecked)
{
    if (isChecked)
        updateState(Checked);
    else
        updateState(Normal);

    QAbstractButton::setChecked(isChecked);
}

void CategoryButton::setTextVisible(bool visible, const bool animation)
{
    if (!animation) {
        m_textLabel->setVisible(visible);
    } else {
        m_textLabel->setVisible(true);
        if (visible) {
            m_opacityAnimation->setStartValue(titleOpacity());
            m_opacityAnimation->setEndValue(1);
        } else {
            m_opacityAnimation->setStartValue(titleOpacity());
            m_opacityAnimation->setEndValue(0);
        }
        m_opacityAnimation->start();
    }
}

void CategoryButton::setInfoByCategory()
{
    switch (m_category)
    {
    case AppsListModel::Internet:       setAccessibleName("internet");
                                        m_textLabel->setText(getCategoryNames("Internet"));
                                        m_iconName = "internet";                    break;
    case AppsListModel::Chat:           setAccessibleName("chat");
                                        m_textLabel->setText(getCategoryNames("Chat"));
                                        m_iconName = "chat";                        break;
    case AppsListModel::Music:          setAccessibleName("music");
                                        m_textLabel->setText(getCategoryNames("Music"));
                                        m_iconName = "music";                       break;
    case AppsListModel::Video:          setAccessibleName("multimedia");
                                        m_textLabel->setText(getCategoryNames("Video"));
                                        m_iconName = "multimedia";                  break;
    case AppsListModel::Graphics:       setAccessibleName("graphics");
                                        m_textLabel->setText(getCategoryNames("Graphics"));
                                        m_iconName = "graphics";                    break;
    case AppsListModel::Game:           setAccessibleName("game");
                                        m_textLabel->setText(getCategoryNames("Game"));
                                        m_iconName = "game";                        break;
    case AppsListModel::Office:         setAccessibleName("office");
                                        m_textLabel->setText(getCategoryNames("Office"));
                                        m_iconName = "office";                      break;
    case AppsListModel::Reading:        setAccessibleName("reading");
                                        m_textLabel->setText(getCategoryNames("Reading"));
                                        m_iconName = "reading";                     break;
    case AppsListModel::Development:    setAccessibleName("development");
                                        m_textLabel->setText(getCategoryNames("Development"));
                                        m_iconName = "development";                 break;
    case AppsListModel::System:         setAccessibleName("system");
                                        m_textLabel->setText(getCategoryNames("System"));
                                        m_iconName = "system";                      break;
//    case AppsListModel::Others:         m_textLabel->setText(tr("Others"));       break;
    default:                            setAccessibleName("others");
                                        m_textLabel->setText(getCategoryNames("Others"));
                                        m_iconName = "others";                      break;
    }
}

void CategoryButton::updateState(const CategoryButton::State state)
{
    if (state == m_state)
        return;
    m_state = state;

    QString picState;
    switch (state)
    {
    case Checked:   picState = "active";    break;
    case Hover:     picState = "hover";     break;
    default:        picState = "normal";    break;
    }

    QPixmap categoryPix;
    categoryPix.load(QString(":/icons/skin/icons/%1_%2_22px.svg").arg(m_iconName).arg(picState));
    categoryPix = categoryPix.scaled(QSize(22, 22) * qApp->devicePixelRatio(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    categoryPix.setDevicePixelRatio(qApp->devicePixelRatio());
    m_iconLabel->setPixmap(categoryPix);

    updateTextColor();
}

void CategoryButton::updateTextColor()
{
    QPalette p = m_textLabel->palette();
    if (m_state == Normal) {
        p.setColor(m_textLabel->foregroundRole(), QColor::fromRgbF(1, 1, 1, m_titleOpacity * 0.6));
    } else if (m_state == Hover) {
        p.setColor(m_textLabel->foregroundRole(), QColor::fromRgbF(1, 1, 1, m_titleOpacity * 0.8));
    } else {
        p.setColor(m_textLabel->foregroundRole(), QColor::fromRgbF(1, 1, 1, m_titleOpacity));
    }
    p.setColor(m_textLabel->backgroundRole(), Qt::transparent);
    m_textLabel->setPalette(p);
}

void CategoryButton::addTextShadow() {
    QGraphicsDropShadowEffect* textDropShadow = new QGraphicsDropShadowEffect;
    textDropShadow->setBlurRadius(4);
    textDropShadow->setColor(QColor(0, 0, 0, 128));
    textDropShadow->setOffset(0, 2);
    m_textLabel->setGraphicsEffect(textDropShadow);
}

void CategoryButton::relayout()
{
//    setStyleSheet(QString("background-color:transparent; color: white; font-size: %1px;")
//                  .arg(m_calcUtil->navgationTextSize()));
    QFont font = m_textLabel->font();
    font.setPixelSize(m_calcUtil->navgationTextSize());
    m_textLabel->setFont(font);
}

qreal CategoryButton::zoomLevel() const
{
    return m_zoomLevel;
}

void CategoryButton::setZoomLevel(const qreal &zoomLevel)
{
    if (m_zoomLevel != zoomLevel) {
        m_zoomLevel = zoomLevel;

        const auto ratio = qApp->devicePixelRatio();

        setFixedHeight(DLauncher::NAVIGATION_ICON_HEIGHT * ratio * zoomLevel);
        m_iconLabel->setFixedSize(22.0 * ratio * zoomLevel, 22.0 * ratio * zoomLevel);

        QFont font = m_textLabel->font();
        font.setPixelSize(m_calcUtil->navgationTextSize() * zoomLevel);
        m_textLabel->setFont(font);
    }
}

qreal CategoryButton::titleOpacity() const
{
    return m_titleOpacity;
}

void CategoryButton::setTitleOpacity(const qreal &titleOpacity)
{
    if (m_titleOpacity != titleOpacity) {
        m_titleOpacity = titleOpacity;

        updateTextColor();
    }
}

QLabel *CategoryButton::textLabel()
{
    return m_textLabel;
}
