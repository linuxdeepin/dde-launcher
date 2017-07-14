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

    Q_PROPERTY(bool active READ active NOTIFY activeChanged)

public:
    explicit MiniCategoryWidget(QWidget *parent = 0);

    inline bool active() const { return m_active; }
    inline AppsListModel::AppCategory currentCategory() const { return m_currentCategory; }

signals:
    void requestRight() const;
    void activeChanged(bool) const;
    void requestCategory(const AppsListModel::AppCategory &category) const;

protected:
    void mousePressEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void enterEvent(QEvent *e);
    bool event(QEvent *event);

private slots:
    void selectNext();
    void selectPrev();

    void onCategoryListChanged();

private:
    bool m_active;
    AppsListModel::AppCategory m_currentCategory;
    AppsManager *m_appsManager;

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
    MiniCategoryItem *m_others;

    QList<MiniCategoryItem *> m_items;
};

#endif // MINICATEGORYWIDGET_H
