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

#include "miniframerightbar.h"
#include "miniframebutton.h"
#include "avatar.h"

#include <DDesktopServices>
#include <QVBoxLayout>
#include <QPainter>
#include <QEvent>
#include <QApplication>
#include <QDesktopWidget>

#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
#include <DDBusSender>
#else
#include <QProcess>
#endif

MiniFrameRightBar::MiniFrameRightBar(QWidget *parent)
    : QWidget(parent)

    , m_modeToggleBtn(new DImageButton(this))
    , m_datetimeWidget(new DatetimeWidget)
    , m_avatar(new Avatar)
    , m_currentIndex(0)
{
    m_modeToggleBtn->setNormalPic(":/icons/skin/icons/fullscreen_normal.png");
    m_modeToggleBtn->setHoverPic(":/icons/skin/icons/fullscreen_hover.png");
    m_modeToggleBtn->setPressPic(":/icons/skin/icons/fullscreen_press.png");
    m_modeToggleBtn->setFixedSize(24, 24);

    m_modeToggleBtn->raise();

    bool hasManual = QProcess::execute("which", QStringList() << "dman") == 0;

    QVBoxLayout *    layout       = new QVBoxLayout(this);
    QHBoxLayout *    bottomLayout = new QHBoxLayout;
    MiniFrameButton *computerBtn  = new MiniFrameButton(tr("Computer"));
    MiniFrameButton *videoBtn     = new MiniFrameButton(tr("Videos"));
    MiniFrameButton *musicBtn     = new MiniFrameButton(tr("Music"));
    MiniFrameButton *pictureBtn   = new MiniFrameButton(tr("Pictures"));
    MiniFrameButton *documentBtn  = new MiniFrameButton(tr("Documents"));
    MiniFrameButton *downloadBtn  = new MiniFrameButton(tr("Downloads"));
    MiniFrameButton *manualBtn    = new MiniFrameButton(tr("Manual"));
    m_settingsBtn                 = new MiniFrameButton(tr("Settings"));
    m_shutdownBtn                 = new MiniFrameButton(tr("Shutdown"));

    manualBtn->setVisible(hasManual);

    uint index = 0;
    m_btns[index++] = computerBtn;
    m_btns[index++] = videoBtn;
    m_btns[index++] = musicBtn;
    m_btns[index++] = pictureBtn;
    m_btns[index++] = documentBtn;
    m_btns[index++] = downloadBtn;

    if (hasManual) {
        m_btns[index++] = manualBtn;
    }

    m_btns[index++] = m_settingsBtn;
    m_btns[index++] = m_shutdownBtn;

    for (auto it = m_btns.constBegin(); it != m_btns.constEnd(); ++it) {
        it.value()->setCheckable(true);
        connect(it.value(), &MiniFrameButton::entered, this, [=] {
            hideAllHoverState();
            m_currentIndex = m_btns.key(it.value());
            setCurrentCheck(true);
        }, Qt::QueuedConnection);
    }

    m_settingsBtn->setIcon(QIcon(":/widgets/images/settings.svg"));
    m_shutdownBtn->setIcon(QIcon(":/widgets/images/power.svg"));

    bottomLayout->addWidget(m_settingsBtn);
    bottomLayout->addWidget(m_shutdownBtn);

    QWidget *top_widget = new QWidget;
    QHBoxLayout *top_layout = new QHBoxLayout;
    top_layout->setMargin(0);
    top_layout->addWidget(m_avatar, 0, Qt::AlignVCenter | Qt::AlignLeft);

    top_widget->setLayout(top_layout);

    QWidget *center_widget = new QWidget;
    QVBoxLayout *center_layout = new QVBoxLayout;
    center_layout->setMargin(0);
    center_widget->setLayout(center_layout);
    center_layout->addWidget(computerBtn);
    center_layout->addWidget(videoBtn);
    center_layout->addWidget(musicBtn);
    center_layout->addWidget(pictureBtn);
    center_layout->addWidget(documentBtn);
    center_layout->addWidget(downloadBtn);
    center_layout->addWidget(manualBtn);

    QWidget *bottom_widget = new QWidget;
    QVBoxLayout *bottom_layout = new QVBoxLayout;
    bottom_layout->setMargin(0);
    bottom_layout->addWidget(m_datetimeWidget);
    bottom_layout->addLayout(bottomLayout);
    bottom_widget->setLayout(bottom_layout);

    layout->setSpacing(0);
    layout->addSpacing(30);
    layout->addWidget(top_widget, 0, Qt::AlignTop);
    layout->addWidget(center_widget, 0, Qt::AlignVCenter);
    layout->addWidget(bottom_widget, 0, Qt::AlignBottom);
    layout->setContentsMargins(18, 0, 12, 18);

    updateSize();

    connect(m_modeToggleBtn, &DImageButton::clicked, this, &MiniFrameRightBar::modeToggleBtnClicked);
    connect(computerBtn, &QPushButton::clicked, this, [this] { openDirectory("computer:///"); });
    connect(documentBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::DocumentsLocation); });
    connect(videoBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::MoviesLocation); });
    connect(musicBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::MusicLocation); });
    connect(pictureBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::PicturesLocation); });
    connect(downloadBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::DownloadLocation); });
    connect(manualBtn, &QPushButton::clicked, this, &MiniFrameRightBar::showManual);
    connect(m_settingsBtn, &QPushButton::clicked, this, &MiniFrameRightBar::showSettings);
    connect(m_shutdownBtn, &QPushButton::clicked, this, &MiniFrameRightBar::showShutdown);
    connect(m_avatar, &Avatar::clicked, this, &MiniFrameRightBar::handleAvatarClicked);
    connect(m_datetimeWidget, &DatetimeWidget::clicked, this, &MiniFrameRightBar::handleTimedateOpen);
}

MiniFrameRightBar::~MiniFrameRightBar()
{
}

void MiniFrameRightBar::setCurrentCheck(bool checked) const
{
    m_btns[m_currentIndex]->setChecked(checked);
}

void MiniFrameRightBar::moveUp()
{
    hideAllHoverState();

    // move to bottom
    if (--m_currentIndex < 0) {
        m_currentIndex = m_btns.size() - 1;
        m_btns.last()->setChecked(true);
        return;
    }

    setCurrentCheck(true);
}

void MiniFrameRightBar::moveDown()
{
    hideAllHoverState();

    // move to head
    if (++m_currentIndex > m_btns.size() - 1) {
        m_currentIndex = 0;
        m_btns.first()->setChecked(true);
        return;
    }

    setCurrentCheck(true);
}

void MiniFrameRightBar::execCurrent()
{
    emit m_btns[m_currentIndex]->clicked();
}

void MiniFrameRightBar::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter painter(this);
    painter.setPen(QColor(255, 255, 255, 0.1 * 255));
    painter.drawLine(QPoint(0, 0),
    QPoint(0, rect().height()));
}

void MiniFrameRightBar::showEvent(QShowEvent *event) {
    return QWidget::showEvent(event);
}

bool MiniFrameRightBar::event(QEvent *event) {
    if (event->type() == QEvent::ApplicationFontChange) {
        updateSize();
    }

    return QWidget::event(event);
}

void MiniFrameRightBar::openDirectory(const QString &dir)
{
    DDesktopServices::showFolder(QUrl(dir));

    emit requestFrameHide();
}

void MiniFrameRightBar::openStandardDirectory(const QStandardPaths::StandardLocation &location)
{
    const QString dir = QStandardPaths::writableLocation(location);

    if (!dir.isEmpty())
        openDirectory(dir);
}

void MiniFrameRightBar::handleShutdownAction(const QString &action)
{
#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
    DDBusSender()
            .service("com.deepin.dde.shutdownFront")
            .interface("com.deepin.dde.shutdownFront")
            .path("/com/deepin/dde/shutdownFront")
            .method(action)
            .call();
#else
    const QString command = QString("dbus-send --print-reply --dest=com.deepin.dde.shutdownFront " \
                                    "/com/deepin/dde/shutdownFront " \
                                    "com.deepin.dde.shutdownFront.%1").arg(action);

    QProcess::startDetached(command);
#endif
    emit requestFrameHide();
}

void MiniFrameRightBar::handleTimedateOpen()
{
#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
    DDBusSender()
            .service("com.deepin.dde.ControlCenter")
            .interface("com.deepin.dde.ControlCenter")
            .path("/com/deepin/dde/ControlCenter")
            .method(QStringLiteral("ShowModule"))
            .arg(QStringLiteral("datetime"))
            .call();
#else
    const QString command("dbus-send "
                          "--type=method_call "
                          "--dest=com.deepin.dde.ControlCenter "
                          "/com/deepin/dde/ControlCenter "
                          "com.deepin.dde.ControlCenter.ShowModule "
                          "string:datetime");

    QProcess::startDetached(command);
#endif
    emit requestFrameHide();
}

void MiniFrameRightBar::handleAvatarClicked()
{
#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
    DDBusSender()
            .service("com.deepin.dde.ControlCenter")
            .interface("com.deepin.dde.ControlCenter")
            .path("/com/deepin/dde/ControlCenter")
            .method(QStringLiteral("ShowModule"))
            .arg(QStringLiteral("accounts"))
            .call();
#else
    const QString command("dbus-send "
                          "--type=method_call "
                          "--dest=com.deepin.dde.ControlCenter "
                          "/com/deepin/dde/ControlCenter "
                          "com.deepin.dde.ControlCenter.ShowModule "
                          "string:accounts");

    QProcess::startDetached(command);
#endif
    emit requestFrameHide();
}

void MiniFrameRightBar::showShutdown()
{
    QProcess::startDetached("dde-shutdown");
}

void MiniFrameRightBar::showSettings()
{
#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
    DDBusSender()
            .service("com.deepin.dde.ControlCenter")
            .interface("com.deepin.dde.ControlCenter")
            .path("/com/deepin/dde/ControlCenter")
            .method(QString("Toggle"))
            .call();
#else
    const QString command("dbus-send "
                          "--type=method_call "
                          "--dest=com.deepin.dde.ControlCenter "
                          "/com/deepin/dde/ControlCenter "
                          "com.deepin.dde.ControlCenter.Toggle");
    QProcess::startDetached(command);
#endif

    emit requestFrameHide();
}

void MiniFrameRightBar::showManual()
{
    QProcess::startDetached("dman");

    emit requestFrameHide();
}

void MiniFrameRightBar::hideAllHoverState() const
{
    for (auto it = m_btns.constBegin(); it != m_btns.constEnd(); ++it) {
        it.value()->setChecked(false);
    }
}

void MiniFrameRightBar::updateSize()
{
    // To calculate width to adaptive width.
    const int padding    = 10;
    const int iconWidth  = 24;
    int       btnWidth   = 0;
    int       frameWidth = 0;

    const int dateTextWidth = m_datetimeWidget->getDateTextWidth() + 60;

    btnWidth += m_settingsBtn->fontMetrics().boundingRect(m_settingsBtn->text()).width() + iconWidth + padding;
    btnWidth += m_shutdownBtn->fontMetrics().boundingRect(m_shutdownBtn->text()).width() + iconWidth + padding;
    btnWidth += 38;  // padding

    if (btnWidth > dateTextWidth) {
        frameWidth = btnWidth;
    }
    else {
        frameWidth = dateTextWidth;
    }

    setFixedWidth(frameWidth);
    m_modeToggleBtn->move(frameWidth - 24 - 5, 12);
}
