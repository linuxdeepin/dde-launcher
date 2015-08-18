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
    int bottomMargin = qApp->desktop()->screenGeometry().height() -
            qApp->desktop()->availableGeometry().height();
    setGeometry(qApp->desktop()->screenGeometry());
    m_navigationBar = new NavigationBar(this);
    m_navigationBar->initUI(leftMargin);

    m_categoryTableWidget = new CategoryTableWidget(this);
    m_categoryTableWidget->setGridParameter(column, gridWidth, itemWidth);

    QVBoxLayout* tableLayout = new QVBoxLayout;
    tableLayout->addWidget(m_categoryTableWidget);
    tableLayout->setContentsMargins(0, 20, 0, bottomMargin);


    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_navigationBar);
    layout->addLayout(tableLayout);
    layout->addStretch();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, rightMargin, 0);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    m_categoryTableWidget->setFocusPolicy(Qt::NoFocus);
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

