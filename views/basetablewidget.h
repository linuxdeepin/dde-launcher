#ifndef BASETABLEWIDGET_H
#define BASETABLEWIDGET_H

#include <QTableWidget>
class QMouseEvent;
class QHideEvent;

class BaseTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit BaseTableWidget(QWidget *parent = 0);
    ~BaseTableWidget();

    void firstHighlight();
    void hightlightItem(int row, int column);

    void handleKeyUpPressed();
    void handleKeyDownPressed();
    void handleKeyLeftPressed();
    void handleKeyRightPressed();

signals:

public slots:
    void clearHighlight();
    void setItemUnChecked();
    void handleDirectionKeyPressed(Qt::Key key);

protected:
    void mouseReleaseEvent(QMouseEvent* event);
    void hideEvent(QHideEvent* event);
    int p_highlightRow = -1;
    int p_highlightColumn = -1;

};

#endif // BASETABLEWIDGET_H
