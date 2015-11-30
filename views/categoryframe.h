#ifndef CATEGORYFRAME_H
#define CATEGORYFRAME_H

#include <QFrame>

class QShowEvent;
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

    QRect topGradientRect() const;
    QRect bottomGradientRect() const;

signals:
    void showed();
    void contentScrolled(int value);

protected:
    void showEvent(QShowEvent *);

private:
    NavigationBar* m_navigationBar;
    CategoryTableWidget* m_categoryTableWidget;
};

#endif // CATEGORYFRAME_H
