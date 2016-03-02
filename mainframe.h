#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "applistview.h"
#include "model/appslistmodel.h"

#include <QFrame>

class MainFrame : public QFrame
{
    Q_OBJECT

public:
    explicit MainFrame(QWidget *parent = 0);

private:
    AppListView *m_customAppsView;
    AppsListModel *m_customAppsModel;
};

#endif // MAINFRAME_H
