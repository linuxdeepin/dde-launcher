#ifndef CATEGORYLISTWIDGET_H
#define CATEGORYLISTWIDGET_H

#include <QWidget>
#include <QStackedLayout>

#include "navigationbuttonframe.h"
class NavigationListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NavigationListWidget(QWidget *parent = 0);
    ~NavigationListWidget();
    void initUI(int width);
    void initConnect();
    NavigationButtonFrame* getIconFrame();
    NavigationButtonFrame* getTextFrame();

signals:
    void currentIndexChanged(int num);
public slots:
    void setCurrentIndex(int index);

private:
    QStackedLayout* m_layout;
    NavigationButtonFrame* m_iconFrame;
    NavigationButtonFrame* m_textFrame;
};

#endif // CATEGORYLISTWIDGET_H
