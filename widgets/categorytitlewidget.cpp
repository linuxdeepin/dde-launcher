#include "categorytitlewidget.h"
#include "global_util/constants.h"
#include "global_util/util.h"

#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>

CategoryTitleWidget::CategoryTitleWidget(const QString &title, QWidget *parent) :
    QWidget(parent)
{
    QLabel* whiteLine = new QLabel(this);
    whiteLine->setObjectName("CategoryWhiteLine");
    whiteLine->setFixedHeight(1);

    QVBoxLayout* lineLayout = new QVBoxLayout;
    lineLayout->setMargin(0);
    lineLayout->setSpacing(0);
    lineLayout->addStretch();
    lineLayout->addWidget(whiteLine);
    lineLayout->addStretch();

    m_title = new QLabel(this);

    QString titleContent = getCategoryNames(title);
    m_title->setText(titleContent);

    QFont titleFont;
    titleFont.setPixelSize(20);
    m_title->setFont(titleFont);
    QFontMetrics fontMetric(titleFont);
    int width=fontMetric.width(titleContent);
    m_title->setFixedWidth(width + 10);
    m_title->setStyleSheet("color: white; background-color:transparent;");

    QHBoxLayout *mainLayout = new QHBoxLayout;

    mainLayout->addWidget(m_title);
    mainLayout->addLayout(lineLayout);
    setLayout(mainLayout);
    setFixedHeight(DLauncher::CATEGORY_TITLE_WIDGET_HEIGHT);

    addTextShadow();
    setStyleSheet(getQssFromFile(":/skin/qss/categorytitlewidget.qss"));
}

void CategoryTitleWidget::setTextVisible(const bool visible)
{
    m_title->setVisible(visible);
}

void CategoryTitleWidget::addTextShadow()
{
    QGraphicsDropShadowEffect* textDropShadow = new QGraphicsDropShadowEffect;
    textDropShadow->setBlurRadius(4);
    textDropShadow->setColor(QColor(0, 0, 0, 128));
    textDropShadow->setOffset(0, 2);
    m_title->setGraphicsEffect(textDropShadow);
}

QLabel *CategoryTitleWidget::textLabel()
{
    return m_title;
}
