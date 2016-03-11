#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "delegate/appitemdelegate.h"
#include "global_util/util.h"
#include "model/appslistmodel.h"
#include "view/applistview.h"
#include "categorytitlewidget.h"
#include "navigationlistwidget.h"
#include "searchwidget.h"
#include "maincategoryframe.h"
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
    void initConnect();
    void scrollToCategory(int value);

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
    AppsListModel *m_customAppsModel;
    AppItemDelegate *m_appItemDelegate;
    MainCategoryFrame* m_mainCategoryFrame;
};

#endif // MAINFRAME_H

