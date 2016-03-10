#ifndef MAINCATEGORYFRAME_H
#define MAINCATEGORYFRAME_H

#include <QFrame>
#include <QVBoxLayout>

#include "model/appslistmodel.h"
#include "model/appsmanager.h"
#include "view/applistview.h"
#include "delegate/appitemdelegate.h"
#include "categorytitlewidget.h"

class MainCategoryFrame:public QFrame {
    Q_OBJECT
public:
    MainCategoryFrame(QFrame* parent=0);
    ~MainCategoryFrame();

    void updateUI();
private:
    QVBoxLayout* m_layout;
    QList<AppListView*> m_listViewList;
    QList<AppsListModel*> m_listModelList;
    QList<CategoryTitleWidget*> m_cateTitleWidgetList;
};


#endif // MAINCATEGORYFRAME_H
