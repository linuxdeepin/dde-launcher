#ifndef BASETABLEWIDGET_H
#define BASETABLEWIDGET_H

#include <QTableWidget>
class QMouseEvent;

class BaseTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit BaseTableWidget(QWidget *parent = 0);
    ~BaseTableWidget();

signals:

public slots:
    void setItemUnChecked();

protected:
    void mouseReleaseEvent(QMouseEvent* event);


};

#endif // BASETABLEWIDGET_H
