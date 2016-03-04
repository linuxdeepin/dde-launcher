#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "applistview.h"
#include "appitemdelegate.h"
#include "model/appslistmodel.h"

#include <QFrame>
#include <QScrollArea>

#include <dboxwidget.h>

DWIDGET_USE_NAMESPACE

class MainFrame : public QFrame
{
    Q_OBJECT

public:
    explicit MainFrame(QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent *e);
    bool eventFilter(QObject *o, QEvent *e);

private slots:

private:
    QScrollArea *m_appsArea;
    DVBoxWidget *m_appsVbox;
    AppListView *m_customAppsView;
    AppListView *m_testView;
    AppsListModel *m_customAppsModel;
    AppItemDelegate *m_appItemDelegate;
};

#endif // MAINFRAME_H
