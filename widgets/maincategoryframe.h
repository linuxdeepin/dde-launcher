#ifndef MAINCATEGORYFRAME_H
#define MAINCATEGORYFRAME_H

#include <QFrame>
#include <QVBoxLayout>

#include "model/appslistmodel.h"
#include "model/appsmanager.h"
#include "view/applistview.h"
#include "delegate/appitemdelegate.h"
#include "categorytitlewidget.h"
#include "global_util/global.h"
#include "global_util/signalmanager.h"
#include "global_util/calcuateutil.h"

class MainCategoryFrame:public QFrame {
    Q_OBJECT
public:
    MainCategoryFrame(QFrame* parent=0);
    ~MainCategoryFrame();

    void updateUI();
    void initConnect();
public slots:
    void scrollToCategory(int index);
    void addAnEmptyFrameInBottom();
private:
    QVBoxLayout* m_layout;
    QFrame* m_bottonFrame;
    QList<AppListView*> m_listViewList;
    QList<AppsListModel*> m_listModelList;
    QList<CategoryTitleWidget*> m_cateTitleWidgetList;
};


#endif // MAINCATEGORYFRAME_H
