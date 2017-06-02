#ifndef APPLISTVIEW_H
#define APPLISTVIEW_H

#include <QListView>

class AppListView : public QListView
{
    Q_OBJECT

public:
    explicit AppListView(QWidget *parent = 0);

signals:
    void popupMenuRequested(const QPoint &pos, const QModelIndex &index) const;

protected:
    void mousePressEvent(QMouseEvent *e);
};

#endif // APPLISTVIEW_H
