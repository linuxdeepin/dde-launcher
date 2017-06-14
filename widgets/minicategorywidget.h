#ifndef MINICATEGORYWIDGET_H
#define MINICATEGORYWIDGET_H

#include <QWidget>
#include <QPushButton>

#include "model/appslistmodel.h"

class MiniCategoryItem : public QPushButton
{
    Q_OBJECT

public:
    explicit MiniCategoryItem(const QString &title, QWidget *parent = Q_NULLPTR);
};

class MiniCategoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MiniCategoryWidget(QWidget *parent = 0);

signals:
    void requestCategory(const AppsListModel::AppCategory &category) const;

protected:
    void mousePressEvent(QMouseEvent *e);

private:
    QButtonGroup *m_buttonGroup;

    MiniCategoryItem *m_allApps;
    MiniCategoryItem *m_internet;
    MiniCategoryItem *m_chat;
    MiniCategoryItem *m_music;
    MiniCategoryItem *m_video;
    MiniCategoryItem *m_graphics;
    MiniCategoryItem *m_game;
    MiniCategoryItem *m_office;
    MiniCategoryItem *m_reading;
    MiniCategoryItem *m_development;
    MiniCategoryItem *m_system;
};

#endif // MINICATEGORYWIDGET_H
