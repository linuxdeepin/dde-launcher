#include "categoryitem.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

CategoryItem::CategoryItem(QString text, QWidget *parent) : QFrame(parent)
{
    initUI(text);
}

void CategoryItem::initUI(QString text){
    QLabel* label = new QLabel(text, this);
    label->setStyleSheet("color: white;\
                         font-size: 20px;");
    QFont font;
    font.setPixelSize(20);
    label->setFont(font);
    label->setFixedWidth(label->fontMetrics().width(text));
//    label->setObjectName("CategoryItemText");
    QLabel* blackLineLabel = new QLabel;
    blackLineLabel->setObjectName("CategoryItemBlackLine");
    QLabel* whiteLineLabel = new QLabel;
    whiteLineLabel->setObjectName("CategoryItemWhiteLine");
    blackLineLabel->setFixedHeight(1);
    whiteLineLabel->setFixedHeight(1);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(label, Qt::AlignTop);

    QVBoxLayout* lineLayout = new QVBoxLayout;
    lineLayout->addStretch();
    lineLayout->addWidget(blackLineLabel);
    lineLayout->addWidget(whiteLineLabel);
    lineLayout->addStretch();
    lineLayout->setSpacing(0);
    lineLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(lineLayout, Qt::AlignTop);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(5, 0, 0, 0);
    setLayout(mainLayout);
}

CategoryItem::~CategoryItem()
{

}

