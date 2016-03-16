#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include "categorybutton.h"

#include <QWidget>
#include <QButtonGroup>

class NavigationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NavigationWidget(QWidget *parent = 0);

    void setButtonsVisible(const bool visible);

signals:
    void scrollToCategory(const AppsListModel::AppCategory category) const;

public slots:
    void setCurrentCategory(const AppsListModel::AppCategory category);
    void refershCategoryVisible(const AppsListModel::AppCategory category, const int appNums);

private:
    void initUI();
    void initConnection();

    void buttonClicked();
    CategoryButton *button(const AppsListModel::AppCategory category);

private:
    QButtonGroup *m_categoryGroup;

    CategoryButton *m_internetBtn;
    CategoryButton* m_chatBtn;
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
