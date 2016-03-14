#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include "categorybutton.h"

#include <QWidget>

class NavigationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NavigationWidget(QWidget *parent = 0);

signals:
    void scrollToCategory(const AppsListModel::AppCategory category) const;

public slots:
    void setCurrentCategory(const AppsListModel::AppCategory category);

private:
    void initUI();
    void initConnection();

    void buttonClicked();

private:
    CategoryButton *m_internetBtn;
    CategoryButton *m_musicBtn;
    CategoryButton *m_videoBtn;
    CategoryButton *m_graphicsBtn;
    CategoryButton *m_gameBtn;
    CategoryButton *m_officeBtn;
    CategoryButton *m_readingBtn;
    CategoryButton *m_developmentBtn;
    CategoryButton *m_systemBtn;
    CategoryButton *m_othersBtn;
};

#endif // NAVIGATIONWIDGET_H
