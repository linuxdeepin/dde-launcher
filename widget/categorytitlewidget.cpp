#include "categorytitlewidget.h"
#include "global_util/constants.h"
#include "global_util/util.h"

#include <QHBoxLayout>

CategoryTitleWidget::CategoryTitleWidget(const QString &title, QWidget *parent) :
    QWidget(parent),
    m_title(new QLabel(title))
{
    QLabel* blackLine = new QLabel(this);
    blackLine->setObjectName("CategoryBlackLine");
    blackLine->setFixedHeight(1);
    QLabel* whiteLine = new QLabel(this);
    whiteLine->setObjectName("CategoryWhiteLine");
    whiteLine->setFixedHeight(1);

    QVBoxLayout* lineLayout = new QVBoxLayout;
    lineLayout->setMargin(0);
    lineLayout->setSpacing(0);
    lineLayout->addStretch();
    lineLayout->addWidget(blackLine);
    lineLayout->addWidget(whiteLine);
    lineLayout->addStretch();

    QFont titleFont;
    titleFont.setPixelSize(20);
    m_title->setFont(titleFont);

    QFontMetrics fontMetric(titleFont);
    int width=fontMetric.width(title);
    m_title->setFixedWidth(width + 10);
    m_title->setStyleSheet("color: white;");

    QHBoxLayout *mainLayout = new QHBoxLayout;

    mainLayout->addWidget(m_title);
    mainLayout->addLayout(lineLayout);
    setLayout(mainLayout);
    setFixedHeight(DLauncher::CATEGORY_TITLE_WIDGET_HEIGHT);

    setStyleSheet(getQssFromFile(":/skin/qss/categorytitlewidget.qss"));
}
