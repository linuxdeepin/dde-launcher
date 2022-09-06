// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MINIFRAMEBUTTON_H
#define MINIFRAMEBUTTON_H

#include <QPushButton>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

class MiniFrameButton : public QPushButton
{
    Q_OBJECT

public:
    explicit MiniFrameButton(const QString &text, QWidget *parent = nullptr);

signals:
    void entered() const;

public slots:
    void onThemeTypeChanged(DGuiApplicationHelper::ColorType themeType);
    void setIconPath(const QString &path);

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    bool event(QEvent *event) override;

private:
    void updateFont();

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor m_color;
    QPixmap m_icon;
};

#endif
