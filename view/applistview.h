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
    int indexYOffset(const QModelIndex &index) const;

signals:
    void popupMenuRequested(const QPoint &pos, const QModelIndex &index) const;
//    void handleDragItems(const QModelIndex &index, int insertI);

protected:
    using QListView::startDrag;
    void startDrag(const QModelIndex &index);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dropEvent(QDropEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    bool eventFilter(QObject *o, QEvent *e);

private slots:
    void fitToContent();
    void dropSwap();

private:
    int m_dropToPos;
    bool m_enableDropInside = false;
    QPoint m_dragStartPos;

    QTimer *m_dropThresholdTimer;

    static AppsManager *m_appManager;
    static CalculateUtil *m_calcUtil;
};

#endif // APPLISTVIEW_H
