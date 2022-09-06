// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DATETIMEWIDGET_H
#define DATETIMEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>

#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

class QDBusInterface;
class DatetimeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DatetimeWidget(QWidget *parent = nullptr);

    int getDateTextWidth();
    void updateTime();

signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent *) Q_DECL_OVERRIDE;

private slots:
    void onThemeTypeChange(DGuiApplicationHelper::ColorType themeType);

private:
    QLabel *m_currentTimeLabel;
    QLabel *m_currentDateLabel;
    QTimer *m_refreshDateTimer;
    QDBusInterface *m_24HourFormatInter;
};

#endif
