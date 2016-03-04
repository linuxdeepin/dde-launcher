#ifndef APPLISTVIEW_H
#define APPLISTVIEW_H

#include <QListView>
#include <QSize>

class AppListView : public QListView
{
    Q_OBJECT

public:
    explicit AppListView(QWidget *parent = 0);

    QSize sizeHint() const;

protected:
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dropEvent(QDropEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);
    void wheelEvent(QWheelEvent *e);
    bool eventFilter(QObject *o, QEvent *e);

private slots:
    void fitToContent();

private:
    QPoint m_dragStartPos;
};

#endif // APPLISTVIEW_H
