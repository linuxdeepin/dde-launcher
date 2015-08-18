#ifndef CATEGORYFRAME_H
#define CATEGORYFRAME_H

#include <QFrame>

class NavigationBar;
class CategoryTableWidget;

class CategoryFrame : public QFrame
{
    Q_OBJECT
public:
    explicit CategoryFrame(QWidget *parent = 0);
    ~CategoryFrame();

    void initUI(int leftMargin, int rightMargin, int column, int itemWidth, int gridWidth);
    NavigationBar* getNavigationBar();
    CategoryTableWidget* getCategoryTabelWidget();
signals:

public slots:

private:
    NavigationBar* m_navigationBar;
    CategoryTableWidget* m_categoryTableWidget;
};

#endif // CATEGORYFRAME_H
