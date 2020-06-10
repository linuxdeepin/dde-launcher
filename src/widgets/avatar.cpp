/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 *
 * Maintainer: rekols <rekols@foxmail.com>
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

#include "avatar.h"

#include <QPaintEvent>
#include <QPainter>
#include <QDebug>

#include <unistd.h>

Avatar::Avatar(QWidget *parent)
    : QWidget(parent)
{
    m_accountsInter = new AccountsInter("com.deepin.daemon.Accounts",
                                        "/com/deepin/daemon/Accounts",
                                        QDBusConnection::systemBus(), this);

    m_userInter = new UserInter("com.deepin.daemon.Accounts",
                                QString("/com/deepin/daemon/Accounts/User%1").arg(getuid()),
                                QDBusConnection::systemBus(), this);

    this->setAccessibleDescription("This is the head image of the Launcher, which can quickly access the account in the control center");
    setFixedSize(32, 32);
    setFilePath(m_userInter->iconFile());

    connect(m_userInter, &UserInter::IconFileChanged, this, &Avatar::setFilePath);
}

void Avatar::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainterPath painterPath;
    painterPath.addRoundedRect(QRect(0, 0, width(), height()), 6, 6);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipPath(painterPath);

    painter.drawPixmap(e->rect(), m_avatarPixmap);
}

void Avatar::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);

    if (e->button() == Qt::LeftButton) {
        emit clicked();
    }
}

void Avatar::setFilePath(const QString &filePath)
{
    const auto ratio = devicePixelRatioF();

    QString avatarPath = filePath;
    if (ratio > 1.0) {
        avatarPath.replace("icons/", "icons/bigger/");
    }

    QUrl url(filePath);
    if (!QFile(url.toLocalFile()).exists()) {
        url = QUrl(filePath);
    }

    m_avatarPixmap = QPixmap(url.toLocalFile()).scaled(size() * ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_avatarPixmap.setDevicePixelRatio(ratio);

    update();
}
