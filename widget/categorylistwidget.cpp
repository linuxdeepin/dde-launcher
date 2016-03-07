#include "categorylistwidget.h"
#include "constants.h"

#include <QVBoxLayout>
#include <QLabel>

CategoryListWidget::CategoryListWidget(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    QLabel *label = new QLabel("left button list");
    mainLayout->addWidget(label);

    setLayout(mainLayout);
    setFixedWidth(DLauncher::CATEGORY_LIST_WIDGET_WIDTH);
}
