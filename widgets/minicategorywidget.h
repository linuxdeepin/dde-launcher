#ifndef MINICATEGORYWIDGET_H
#define MINICATEGORYWIDGET_H

#include <QWidget>
#include <QPushButton>

#include "model/appslistmodel.h"

class MiniCategoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MiniCategoryWidget(QWidget *parent = 0);

signals:
    void requestCategory(const AppsListModel::AppCategory &category) const;

private:
    QPushButton *m_allApps;
    QPushButton *m_internet;
    QPushButton *m_chat;
    QPushButton *m_music;
    QPushButton *m_video;
    QPushButton *m_graphics;
    QPushButton *m_game;
    QPushButton *m_office;
    QPushButton *m_reading;
    QPushButton *m_development;
    QPushButton *m_system;
};

#endif // MINICATEGORYWIDGET_H
