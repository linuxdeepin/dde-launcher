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
#include "../global_util/util.h"

#include <DDesktopServices>
#include <QVBoxLayout>
#include <QPainter>
#include <QEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <DGuiApplicationHelper>
#include <QGSettings>

DGUI_USE_NAMESPACE

#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
#include <DDBusSender>
#else
#include <QProcess>
#endif

//光标跟随主题的DImageButton,临时解决wayland下光标不跟随主题的bug
class ButtonWithThemeCursor : public DImageButton
{
public:
    ButtonWithThemeCursor(QWidget *parent = 0): DImageButton(parent) {
    }

protected:
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE {
        DImageButton::enterEvent(event);
        updateButtonCursor();
    }
    void updateButtonCursor()
    {
        static QCursor *lastArrowCursor = nullptr;
        static QString  lastCursorTheme;
        int lastCursorSize = 0;
        QGSettings gsetting("com.deepin.xsettings", "/com/deepin/xsettings/");
        QString theme = gsetting.get("gtk-cursor-theme-name").toString();
        int cursorSize = gsetting.get("gtk-cursor-theme-size").toInt();
        if (theme != lastCursorTheme || cursorSize != lastCursorSize)
        {
            QCursor *cursor = loadQCursorFromX11Cursor(theme.toStdString().c_str(), "hand2", cursorSize);
            lastCursorTheme = theme;
            lastCursorSize = cursorSize;
            if(cursor != nullptr)
                setCursor(*cursor);
            if (lastArrowCursor != nullptr)
                delete lastArrowCursor;

            lastArrowCursor = cursor;
        }
    }
};

MiniFrameRightBar::MiniFrameRightBar(QWidget *parent)
    : QWidget(parent)
    , m_avatar(new Avatar)
    , m_currentIndex(0)
{
    setFixedWidth(60);
//    m_modeToggleBtn->raise();

    bool hasManual = QProcess::execute("which", QStringList() << "dman") == 0;

    QVBoxLayout *    layout       = new QVBoxLayout(this);
    QVBoxLayout *    bottomLayout = new QVBoxLayout;
    MiniFrameButton *computerBtn  = new MiniFrameButton(tr(""));
    computerBtn->setFixedSize(16, 16);
    MiniFrameButton *videoBtn     = new MiniFrameButton(tr(""));
    videoBtn->setFixedSize(16, 16);
    MiniFrameButton *musicBtn     = new MiniFrameButton(tr(""));
    musicBtn->setFixedSize(16, 16);
    MiniFrameButton *pictureBtn   = new MiniFrameButton(tr(""));
    pictureBtn->setFixedSize(16, 16);
    MiniFrameButton *documentBtn  = new MiniFrameButton(tr(""));
    documentBtn->setFixedSize(16, 16);
    MiniFrameButton *downloadBtn  = new MiniFrameButton(tr(""));
    downloadBtn->setFixedSize(16, 16);
    m_settingsBtn                 = new MiniFrameButton(tr(""));
    m_settingsBtn->setFixedSize(18, 18);
    m_powerBtn                    = new MiniFrameButton(tr(""));
    m_powerBtn->setFixedSize(18, 18);

    uint index = 0;
    m_btns[index++] = computerBtn;
    m_btns[index++] = videoBtn;
    m_btns[index++] = musicBtn;
    m_btns[index++] = pictureBtn;
    m_btns[index++] = documentBtn;
    m_btns[index++] = downloadBtn;

    m_btns[index++] = m_settingsBtn;
    m_btns[index++] = m_powerBtn;

    for (auto it = m_btns.constBegin(); it != m_btns.constEnd(); ++it) {
        it.value()->setCheckable(true);
        connect(it.value(), &MiniFrameButton::entered, this, [=] {
            hideAllHoverState();
            m_currentIndex = m_btns.key(it.value());
            setCurrentCheck(true);
        }, Qt::QueuedConnection);
    }

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [ = ](DGuiApplicationHelper::ColorType themeType) {
        if (DGuiApplicationHelper::LightType == themeType) {
            m_settingsBtn->setIcon(QIcon(":/widgets/images/setting_dark.svg"));
            m_powerBtn->setIcon(QIcon(":/widgets/images/shutdown_dark.svg"));
            computerBtn->setIcon(QIcon(":/widgets/images/computer-symbolic_dark"));
            videoBtn->setIcon(QIcon(":/widgets/images/folder-videos-symbolic_dark"));
            musicBtn->setIcon(QIcon(":/widgets/images/folder-music-symbolic_dark"));
            pictureBtn->setIcon(QIcon(":/widgets/images/folder-pictures-symbolic_dark"));
            documentBtn->setIcon(QIcon(":/widgets/images/folder-documents-symbolic_dark"));
            downloadBtn->setIcon(QIcon(":/widgets/images/folder-downloads-symbolic_dark"));
        } else {
            m_settingsBtn->setIcon(QIcon(":/widgets/images/setting.svg"));
            m_powerBtn->setIcon(QIcon(":/widgets/images/shutdown.svg"));
            computerBtn->setIcon(QIcon(":/widgets/images/computer-symbolic"));
            videoBtn->setIcon(QIcon(":/widgets/images/folder-videos-symbolic"));
            musicBtn->setIcon(QIcon(":/widgets/images/folder-music-symbolic"));
            pictureBtn->setIcon(QIcon(":/widgets/images/folder-pictures-symbolic"));
            documentBtn->setIcon(QIcon(":/widgets/images/folder-documents-symbolic"));
            downloadBtn->setIcon(QIcon(":/widgets/images/folder-downloads-symbolic"));
        }
    });

    if (DGuiApplicationHelper::LightType ==DGuiApplicationHelper::instance()->themeType()) {
        m_settingsBtn->setIcon(QIcon(":/widgets/images/settings-dark.svg"));
        m_powerBtn->setIcon(QIcon(":/widgets/images/power-dark.svg"));
    } else {
        m_settingsBtn->setIcon(QIcon(":/widgets/images/settings.svg"));
        m_powerBtn->setIcon(QIcon(":/widgets/images/power.svg"));
    }

    bottomLayout->addWidget(m_settingsBtn, 0, Qt::AlignCenter);
    bottomLayout->addWidget(m_powerBtn,0 ,Qt::AlignCenter);
    bottomLayout->setSpacing(18);

    QWidget *top_widget = new QWidget;
    QHBoxLayout *top_layout = new QHBoxLayout;
    top_layout->setMargin(0);
    top_layout->addWidget(m_avatar, 0, Qt::AlignVCenter | Qt::AlignLeft);

    top_widget->setLayout(top_layout);

    QWidget *center_widget = new QWidget;
    QVBoxLayout *center_layout = new QVBoxLayout;
    center_layout->setMargin(0);
    center_widget->setLayout(center_layout);
    center_layout->setSpacing(24);
    center_layout->addWidget(m_avatar, 0, Qt::AlignCenter);
    center_layout->addWidget(computerBtn, 0, Qt::AlignCenter);
    center_layout->addWidget(documentBtn, 0, Qt::AlignCenter);
    center_layout->addWidget(pictureBtn, 0, Qt::AlignCenter);
    center_layout->addWidget(musicBtn, 0, Qt::AlignCenter);
    center_layout->addWidget(videoBtn, 0, Qt::AlignCenter);
    center_layout->addWidget(downloadBtn, 0, Qt::AlignCenter);

    QWidget *bottom_widget = new QWidget;
    QVBoxLayout *bottom_layout = new QVBoxLayout;
    bottom_layout->setMargin(0);
    bottom_layout->addLayout(bottomLayout);
    bottom_widget->setLayout(bottom_layout);

    layout->addSpacing(18);
    layout->addWidget(center_widget);
    layout->addWidget(bottom_widget, 0, Qt::AlignBottom);
    layout->setContentsMargins(0, 0, 0, 14);

    connect(computerBtn, &QPushButton::clicked, this, [this] { openDirectory("computer:///"); });
    connect(documentBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::DocumentsLocation); });
    connect(videoBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::MoviesLocation); });
    connect(musicBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::MusicLocation); });
    connect(pictureBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::PicturesLocation); });
    connect(downloadBtn, &QPushButton::clicked, this, [this] { openStandardDirectory(QStandardPaths::DownloadLocation); });
    connect(m_settingsBtn, &QPushButton::clicked, this, &MiniFrameRightBar::showSettings);
    connect(m_powerBtn, &QPushButton::clicked, this, &MiniFrameRightBar::showShutdown);
    connect(m_avatar, &Avatar::clicked, this, &MiniFrameRightBar::handleAvatarClicked);

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

void MiniFrameRightBar::showEvent(QShowEvent *event) {
    return QWidget::showEvent(event);
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
    QProcess::startDetached("dbus-send --print-reply --dest=com.deepin.dde.shutdownFront /com/deepin/dde/shutdownFront com.deepin.dde.shutdownFront.Show");

    emit requestFrameHide();
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

