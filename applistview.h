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
    void updatea();

protected:
    void resizeEvent(QResizeEvent *e);
    void wheelEvent(QWheelEvent *e);
};

#endif // APPLISTVIEW_H
