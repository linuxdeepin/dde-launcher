#ifndef APPLISTVIEW_H
#define APPLISTVIEW_H

#include <QListView>
#include <QSize>
#include <QLabel>

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
    void setContainerBox(const QWidget *container);

signals:
    void popupMenuRequested(const QPoint &pos, const QModelIndex &index) const;
    void requestScrollUp() const;
    void requestScrollDown() const;
    void requestScrollStop() const;

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
    void prepareDropSwap();
    void createFakeAnimation(const int pos, const bool moveNext, const bool isLastAni = false);
    void dropSwap();

private:
    const QRect indexRect(const QModelIndex &index) const;

private:
    int m_dropToPos;
    bool m_enableDropInside = false;
    QPoint m_dragStartPos;

    const QWidget *m_containerBox = nullptr;
    QTimer *m_dropThresholdTimer;
    QPropertyAnimation *m_lastFakeAni = nullptr;

    static QPointer<AppsManager> m_appManager;
    static QPointer<CalculateUtil> m_calcUtil;
};

#endif // APPLISTVIEW_H
