// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MINIFRAMERIGHTBAR_H
#define MINIFRAMERIGHTBAR_H

#include <QWidget>
#include <QLabel>

#include <DGuiApplicationHelper>

#include <diconbutton.h>
#include "avatar.h"

DWIDGET_USE_NAMESPACE

class MiniFrameButton;
class MiniFrameRightBar : public QWidget
{
    Q_OBJECT

public:
    explicit MiniFrameRightBar(QWidget *parent = nullptr);
    ~MiniFrameRightBar() override;

    void setCurrentCheck(bool checked) const;
    void setCurrentIndex(int index) { m_currentIndex = index; }
    void moveUp();
    void moveDown();
    void execCurrent();
    void hideAllHoverState() const;

signals:
    void modeToggleBtnClicked();
    void requestFrameHide();

protected:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void openDirectory(const QString &dir);
    void openStandardDirectory(const QStandardPaths::StandardLocation &location);
    void handleShutdownAction(const QString &action);
    void handleAvatarClicked();
    void handleTimedateOpen();
    void showShutdown();
    void showSettings();
    void showManual();
   // void hideAllHoverState() const;
    void updateIcon();

private:
    Avatar *m_avatar;
    int m_currentIndex;
    QStringList m_hideList;
    bool m_hasCompterIcon;
    bool m_hasDocumentIcon;
    bool m_hasPictureIcon;
    bool m_hasMusicIcon;
    bool m_hasVideoIcon;
    bool m_hasDownloadIcon;
    QMap<uint, MiniFrameButton*> m_btns;
    MiniFrameButton *m_settingsBtn;
    MiniFrameButton *m_powerBtn;
    MiniFrameButton *m_computerBtn;
    MiniFrameButton *m_videoBtn;
    MiniFrameButton *m_musicBtn;
    MiniFrameButton *m_pictureBtn;
    MiniFrameButton *m_documentBtn;
    MiniFrameButton *m_downloadBtn;
};

#endif // MINIFRAMERIGHTBAR_H
