#ifndef APPLISTVIEW_H
#define APPLISTVIEW_H

#include <QListView>
#include <QSize>

#include "model/appsmanager.h"

class CalculateUtil;
class AppsListModel;
class AppListView : public QListView
{
    Q_OBJECT

public:
    explicit AppListView(QWidget *parent = 0);

    using QListView::indexAt;
    const QModelIndex indexAt(const int index) const;

signals:
    void popupMenuRequested(const QPoint &pos, const QModelIndex &index) const;
//    void appBeDraged(const QModelIndex &index) const;
//    void appDropedIn(const QModelIndex &index) const;

protected:
    using QListView::startDrag;
    void startDrag(const QModelIndex &index);
    void enterEvent(QEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dropEvent(QDropEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);
    void wheelEvent(QWheelEvent *e);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    bool eventFilter(QObject *o, QEvent *e);

private slots:
    void fitToContent();

private:
    int m_dropToPos;
    QPoint m_dragStartPos;

    static AppsManager *m_appManager;
    static CalculateUtil *m_calcUtil;
};

#endif // APPLISTVIEW_H
