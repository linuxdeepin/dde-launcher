#include "categorytitlewidget.h"
#include "global_util/constants.h"
#include "global_util/util.h"

#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>

CategoryTitleWidget::CategoryTitleWidget(const QString &title, QWidget *parent) :
    QFrame(parent),
    m_calcUtil(CalculateUtil::instance()),
    m_title(new QLabel(this)),
    m_opacityAnimation(new QPropertyAnimation(this, "titleOpacity"))
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

    setText(title);
    setTitleOpacity(1);  // update the style of this widget by force.
    m_opacityAnimation->setDuration(300);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_title);
    mainLayout->addLayout(lineLayout);

    setAccessibleName(title);
    setLayout(mainLayout);
    setFixedHeight(DLauncher::CATEGORY_TITLE_WIDGET_HEIGHT);

    addTextShadow();

    setStyleSheet("QLabel#CategoryWhiteLine {"
                  "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(255,255,255,0.3), stop:1 rgba(255,255,255,0))"
                  "}");

    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, &CategoryTitleWidget::relayout);
}

void CategoryTitleWidget::setTextVisible(const bool visible, const bool animation)
{
    m_opacityAnimation->stop();

    if (!animation) {
        setTitleOpacity(visible ? 1 : 0);
    } else {
        if (visible) {
            m_opacityAnimation->setStartValue(titleOpacity());
            m_opacityAnimation->setEndValue(1);
        } else {
            m_opacityAnimation->setStartValue(titleOpacity());
            m_opacityAnimation->setEndValue(0);
        }
        m_opacityAnimation->start();
    }
}

void CategoryTitleWidget::setText(const QString &title)
{
    QString titleContent = getCategoryNames(title);
    QFontMetrics fontMetric(m_title->font());
    const int width = fontMetric.width(titleContent);
    m_title->setFixedWidth(width + 10);

    // DON'T set style sheet for m_title, otherwise we can't fake opacity
    // animation on it.
    /*
    m_title->setStyleSheet("color: white; background-color:transparent;");
    */

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

qreal CategoryTitleWidget::titleOpacity() const
{
    return m_titleOpacity;
}

void CategoryTitleWidget::setTitleOpacity(const qreal &titleOpacity)
{
    if (m_titleOpacity != titleOpacity) {
        m_titleOpacity = titleOpacity;

        QPalette p = m_title->palette();
        p.setColor(m_title->foregroundRole(), QColor::fromRgbF(1, 1, 1, m_titleOpacity));
        p.setColor(m_title->backgroundRole(), Qt::transparent);
        m_title->setPalette(p);
    }
}

QLabel *CategoryTitleWidget::textLabel()
{
    return m_title;
}
