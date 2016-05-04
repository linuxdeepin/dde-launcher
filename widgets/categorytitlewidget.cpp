#include "categorytitlewidget.h"
#include "global_util/constants.h"
#include "global_util/util.h"

#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>

CategoryTitleWidget::CategoryTitleWidget(const QString &title, QWidget *parent) :
    QFrame(parent),
    m_calcUtil(CalculateUtil::instance(this))
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
    setText(title);
    QHBoxLayout *mainLayout = new QHBoxLayout;

    mainLayout->addWidget(m_title);
    mainLayout->addLayout(lineLayout);
    setLayout(mainLayout);
    setFixedHeight(DLauncher::CATEGORY_TITLE_WIDGET_HEIGHT);

    addTextShadow();

    setStyleSheet("QLabel#CategoryWhiteLine {"
                  "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(255,255,255,0.3), stop:1 rgba(255,255,255,0))"
                  "}");

    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, &CategoryTitleWidget::relayout);
}

void CategoryTitleWidget::setTextVisible(const bool visible)
{
    m_title->setVisible(visible);
}

void CategoryTitleWidget::setText(const QString &title)
{
    QString titleContent = getCategoryNames(title);
    QFontMetrics fontMetric(m_title->font());
    const int width = fontMetric.width(titleContent);
    m_title->setFixedWidth(width + 10);

    m_title->setStyleSheet("color: white; background-color:transparent;");

    m_title->setText(titleContent);
}

void CategoryTitleWidget::addTextShadow()
{
    QGraphicsDropShadowEffect* textDropShadow = new QGraphicsDropShadowEffect;
    textDropShadow->setBlurRadius(4);
    textDropShadow->setColor(QColor(0, 0, 0, 128));
    textDropShadow->setOffset(0, 2);
    m_title->setGraphicsEffect(textDropShadow);
}

void CategoryTitleWidget::relayout()
{
    QFont titleFont(m_title->font());
    titleFont.setPixelSize(m_calcUtil->titleTextSize());
    m_title->setFont(titleFont);
    QFontMetrics fontMetric(titleFont);
    const int width = fontMetric.width(m_title->text());
    m_title->setFixedWidth(width + 10);
}

QLabel *CategoryTitleWidget::textLabel()
{
    return m_title;
}
