// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "miniframebutton.h"

#include <QDebug>
#include <QEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QPainterPath>

DGUI_USE_NAMESPACE

MiniFrameButton::MiniFrameButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    setFocusPolicy(Qt::NoFocus);
    setObjectName("MiniFrameButton");
    setFlat(true);
    setToolTip(text);
    setToolTipDuration(1000);

    onThemeTypeChanged(DGuiApplicationHelper::instance()->themeType());
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &MiniFrameButton::onThemeTypeChanged);
}

void MiniFrameButton::onThemeTypeChanged(DGuiApplicationHelper::ColorType themeType)
{
    if (DGuiApplicationHelper::DarkType == themeType)
        m_color.setRgb(255, 255, 255, 25);
    else
        m_color.setRgb(0, 0, 0, 25);

    update();
}

void MiniFrameButton::setIconPath(const QString &path)
{
    if (!path.isEmpty()) {
        m_icon = QPixmap(path);
    }
}

void MiniFrameButton::enterEvent(QEvent *event)
{
    QPushButton::enterEvent(event);

    setChecked(true);

    emit entered();
}

void MiniFrameButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);

    setChecked(false);
}

bool MiniFrameButton::event(QEvent *event)
{
    if (event->type() == QEvent::ApplicationFontChange) {
        updateFont();
    }

    return QPushButton::event(event);
}

void MiniFrameButton::updateFont()
{
    QFont font = this->font();
    const int px = (qApp->font().pointSizeF() * qApp->desktop()->logicalDpiX() / 72) + 2;
    font.setPixelSize(std::max(px, 14));
    qDebug() << px;
    setFont(font);
}

void MiniFrameButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (isChecked()) {
        QPainterPath path;
        path.addRoundedRect(rect(), 8, 8);
        painter.fillPath(path, m_color);
    }

    if (!m_icon.isNull()) {
        const QPixmap scaled_pixmap = m_icon.scaled(iconSize().width(), iconSize().height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(rect().center().x() - iconSize().width() / 2 + 1 , rect().center().y() - iconSize().height() / 2 + 1, scaled_pixmap);
    }
}
