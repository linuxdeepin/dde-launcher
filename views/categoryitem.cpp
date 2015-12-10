#include "categoryitem.h"
#include "app/global.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QGraphicsDropShadowEffect>

CategoryItem::CategoryItem(QString text, QWidget *parent) : QFrame(parent)
{
    initUI(text);
    installEventFilter(this);
}

void CategoryItem::initUI(QString text){
    int index = CategroyKeys.indexOf(text);
    m_titleLabel = new QLabel(QCoreApplication::translate("QObject", CategoryNames.at(index).toLatin1()), this);
    m_titleLabel->setStyleSheet("color: white");
    QFont font;
    font.setPixelSize(20);
    m_titleLabel->setFont(font);
    m_titleLabel->setFixedWidth(m_titleLabel->fontMetrics().width(m_titleLabel->text()) + 10);
//    label->setObjectName("CategoryItemText");
    QLabel* blackLineLabel = new QLabel;
    blackLineLabel->setObjectName("CategoryItemBlackLine");
    QLabel* whiteLineLabel = new QLabel;
    whiteLineLabel->setObjectName("CategoryItemWhiteLine");
    blackLineLabel->setFixedHeight(1);
    whiteLineLabel->setFixedHeight(1);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_titleLabel, Qt::AlignTop);

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

    addTextShadow();
}

void CategoryItem::addTextShadow(){
    QGraphicsDropShadowEffect *textShadow = new QGraphicsDropShadowEffect;
    textShadow->setBlurRadius(4);
    textShadow->setColor(QColor(0, 0, 0, 128));
    textShadow->setOffset(0, 2);
    m_titleLabel->setGraphicsEffect(textShadow);
}

bool CategoryItem::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::DeferredDelete){
        return true;
    }
    return QFrame::eventFilter(obj, event);
}

CategoryItem::~CategoryItem()
{

}

