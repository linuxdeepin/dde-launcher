#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "appitemdelegate.h"
#include "global_util/util.h"
#include "model/appslistmodel.h"
#include "view/applistview.h"
#include "widget/categorytitlewidget.h"
#include "widget/navigationlistwidget.h"
#include "widget/searchwidget.h"

#include <QFrame>
#include <QScrollArea>

#include <dboxwidget.h>

DWIDGET_USE_NAMESPACE

class MainFrame : public QFrame
{
    Q_OBJECT

public:
    explicit MainFrame(QWidget *parent = 0);

public slots:
    void scrollToCategory(const AppsListModel::AppCategory &category);

protected:
    void resizeEvent(QResizeEvent *e);
    void keyPressEvent(QKeyEvent *e);
    bool eventFilter(QObject *o, QEvent *e);

private slots:

private:

    NavigationListWidget *m_navigationListWidget;
    SearchWidget *m_searchWidget;
    QScrollArea *m_appsArea;
    DVBoxWidget *m_appsVbox;
    AppListView *m_customAppsView;
    AppListView *m_testView;
    AppsListModel *m_customAppsModel;
    AppItemDelegate *m_appItemDelegate;
    CategoryTitleWidget *m_internetTitle;
    CategoryTitleWidget *m_musicTitle;
};

#endif // MAINFRAME_H
