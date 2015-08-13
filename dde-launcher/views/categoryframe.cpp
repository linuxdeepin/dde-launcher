#include "categoryframe.h"

#include "categorytablewidget.h"
#include "navigationbar.h"
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>

CategoryFrame::CategoryFrame(QWidget *parent) : QFrame(parent)
{

}

void CategoryFrame::initUI(int leftMargin, int rightMargin, int column, int itemWidth, int gridWidth){
    setGeometry(qApp->desktop()->screenGeometry());
    m_navigationBar = new NavigationBar(this);
    m_navigationBar->initUI(leftMargin);
    m_categoryTableWidget = new CategoryTableWidget(this);
    m_categoryTableWidget->setGridParameter(column, gridWidth, itemWidth);
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_navigationBar);
    layout->addWidget(m_categoryTableWidget);
    layout->setSpacing(0);
    int bottomMargin = qApp->desktop()->screenGeometry().height() - qApp->desktop()->availableGeometry().height();
    layout->setContentsMargins(0, bottomMargin, rightMargin, bottomMargin);
    setLayout(layout);
}

NavigationBar* CategoryFrame::getNavigationBar(){
    return m_navigationBar;
}

CategoryTableWidget* CategoryFrame::getCategoryTabelWidget(){
    return m_categoryTableWidget;
}


CategoryFrame::~CategoryFrame()
{

}

