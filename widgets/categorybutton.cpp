#include "categorybutton.h"
#include "global_util/constants.h"
#include "global_util/util.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QGraphicsDropShadowEffect>

CategoryButton::CategoryButton(const AppsListModel::AppCategory category, QWidget *parent) :
    QAbstractButton(parent),
    m_category(category),
    m_calcUtil(CalculateUtil::instance(this)),
    m_iconLabel(new QLabel),
    m_textLabel(new QLabel)
{
    setObjectName("CategoryButton");
    m_iconLabel->setFixedSize(22*m_calcUtil->viewMarginRation(), 22*m_calcUtil->viewMarginRation());
    m_textLabel->setFixedHeight(22*m_calcUtil->viewMarginRation());
    m_textLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(m_iconLabel);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(m_textLabel);
    mainLayout->addStretch(3);

    setLayout(mainLayout);
    setCheckable(true);
    setAutoExclusive(true);
    setFocusPolicy(Qt::NoFocus);
    setFixedHeight(DLauncher::NAVIGATION_ICON_HEIGHT);
    setInfoByCategory();
    setStyleSheet(QString("background-color:transparent; color: white; font-size: %1px;").arg(int(15*m_calcUtil->viewMarginRation())));
    updateState(Normal);
    addTextShadow();

    connect(this, &CategoryButton::toggled, this, &CategoryButton::setChecked);
}

AppsListModel::AppCategory CategoryButton::category() const
{
    return m_category;
}

void CategoryButton::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
}

void CategoryButton::enterEvent(QEvent *e)
{
    if (!isChecked())
        updateState(Hover);

    QAbstractButton::enterEvent(e);
}

void CategoryButton::leaveEvent(QEvent *e)
{
    if (!isChecked())
        updateState(Normal);

    QAbstractButton::leaveEvent(e);
}

void CategoryButton::setChecked(bool isChecked)
{
    if (isChecked)
        updateState(Checked);
    else
        updateState(Normal);

    QAbstractButton::setChecked(isChecked);
}

void CategoryButton::setTextVisible(bool visible)
{
    m_textLabel->setVisible(visible);
}

void CategoryButton::setInfoByCategory()
{
    switch (m_category)
    {
    case AppsListModel::Internet:       m_textLabel->setText(getCategoryNames("Internet"));
                                        m_iconName = "internet";                    break;
    case AppsListModel::Chat:           m_textLabel->setText(getCategoryNames("Chat"));
                                        m_iconName = "chat";                        break;
    case AppsListModel::Music:          m_textLabel->setText(getCategoryNames("Music"));
                                        m_iconName = "music";                       break;
    case AppsListModel::Video:          m_textLabel->setText(getCategoryNames("Video"));
                                        m_iconName = "multimedia";                  break;
    case AppsListModel::Graphics:       m_textLabel->setText(getCategoryNames("Graphics"));
                                        m_iconName = "graphics";                    break;
    case AppsListModel::Game:           m_textLabel->setText(getCategoryNames("Game"));
                                        m_iconName = "game";                        break;
    case AppsListModel::Office:         m_textLabel->setText(getCategoryNames("Office"));
                                        m_iconName = "office";                      break;
    case AppsListModel::Reading:        m_textLabel->setText(getCategoryNames("Reading"));
                                        m_iconName = "reading";                     break;
    case AppsListModel::Development:    m_textLabel->setText(getCategoryNames("Development"));
                                        m_iconName = "development";                 break;
    case AppsListModel::System:         m_textLabel->setText(getCategoryNames("System"));
                                        m_iconName = "system";                      break;
//    case AppsListModel::Others:         m_textLabel->setText(tr("Others"));       break;
    default:                            m_textLabel->setText(getCategoryNames("Others"));
                                        m_iconName = "others";                      break;
    }
}

void CategoryButton::updateState(const CategoryButton::State state)
{
    if (state == m_state)
        return;
    m_state = state;

    QString picState;
    switch (state)
    {
    case Checked:   picState = "active";    break;
    case Hover:     picState = "hover";     break;
    default:        picState = "normal";    break;
    }

    QPixmap tmpCategoryMap;
    tmpCategoryMap.load(QString(":/icons/skin/icons/%1_%2_22px.svg").arg(m_iconName).arg(picState));
    m_iconLabel->setPixmap(tmpCategoryMap.scaled(QSize(22*m_calcUtil->viewMarginRation(), 22*m_calcUtil->viewMarginRation()), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void CategoryButton::addTextShadow() {
    QGraphicsDropShadowEffect* textDropShadow = new QGraphicsDropShadowEffect;
    textDropShadow->setBlurRadius(4);
    textDropShadow->setColor(QColor(0, 0, 0, 128));
    textDropShadow->setOffset(0, 2);
    m_textLabel->setGraphicsEffect(textDropShadow);
}

QLabel *CategoryButton::textLabel()
{
    return m_textLabel;
}
