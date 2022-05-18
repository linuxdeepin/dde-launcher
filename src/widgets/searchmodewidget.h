#ifndef SEARCHMODEWIDGET_H
#define SEARCHMODEWIDGET_H

#include <QWidget>
#include <QLabel>

#include <appgridview.h>
#include "appslistmodel.h"
#include "appitemdelegate.h"
#include "multipagesview.h"

#include "dbuslauncher.h"

class SearchModeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchModeWidget(QWidget *parent = Q_NULLPTR);
    ~SearchModeWidget() Q_DECL_OVERRIDE;

    void initUi();
    void initTitle();
    void initAppView();
    void initConnection();
    void setItemDelegate(AppItemDelegate *delegate);

    void setSearchModel(QSortFilterProxyModel *model);
    void updateTitleContent();
    void updateTitlePos(bool alignCenter);
    void addSpacerItem(QBoxLayout *layout = nullptr);

signals:
    void connectViewEvent(AppGridView* pView);

private:
    QLabel *m_nativeLabel;
    QLabel *m_outsideLabel;

    AppGridView *m_nativeView;
    AppGridView *m_outsideView;

    AppsListModel *m_nativeModel;
    AppsListModel *m_outsideModel;

    QLabel *m_emptyIcon;
    QLabel *m_emptyText;

    QWidget *m_nativeWidget;
    QWidget *m_outsideWidget;
    QWidget *m_emptyWidget;

    DBusLauncher *m_launcherInter;
};


#endif