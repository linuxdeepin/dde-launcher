#include "categorybutton.h"
#include "global_util/constants.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QGraphicsDropShadowEffect>

CategoryButton::CategoryButton(const AppsListModel::AppCategory category, QWidget *parent) :
    QAbstractButton(parent),
    m_category(category),
    m_iconLabel(new QLabel),
    m_textLabel(new QLabel)
{
    setObjectName("CategoryButton");

    m_iconLabel->setFixedSize(32, 32);
    m_textLabel->setFixedHeight(32);

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
    setStyleSheet("background-color:transparent; color: white; font-size: 15px;");
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

void CategoryButton::setInfoByCategory()
{
    switch (m_category)
    {
    case AppsListModel::Internet:       m_textLabel->setText(tr("Internet"));
                                        m_iconName = "internet";                    break;
    case AppsListModel::Chat:           m_textLabel->setText(tr("Chat"));
                                        m_iconName = "chat";                        break;
    case AppsListModel::Music:          m_textLabel->setText(tr("Music"));
                                        m_iconName = "music";                       break;
    case AppsListModel::Video:          m_textLabel->setText(tr("Video"));
                                        m_iconName = "video";                       break;
    case AppsListModel::Graphics:       m_textLabel->setText(tr("Graphics"));
                                        m_iconName = "graphics";                    break;
    case AppsListModel::Game:           m_textLabel->setText(tr("Game"));
                                        m_iconName = "game";                        break;
    case AppsListModel::Office:         m_textLabel->setText(tr("Office"));
                                        m_iconName = "office";                      break;
    case AppsListModel::Reading:        m_textLabel->setText(tr("Reading"));
                                        m_iconName = "reading";                     break;
    case AppsListModel::Development:    m_textLabel->setText(tr("Development"));
                                        m_iconName = "development";                 break;
    case AppsListModel::System:         m_textLabel->setText(tr("System"));
                                        m_iconName = "system";                      break;
//    case AppsListModel::Others:         m_textLabel->setText(tr("Others"));       break;
    default:                            m_textLabel->setText(tr("Others"));
                                        m_iconName = "others";                      break;
    }
}

void CategoryButton::updateState(const CategoryButton::State state)
{
    if (state == m_state)
        return;
    m_state = state;

    int picState;
    switch (state)
    {
    case Checked:   picState = 100;    break;
    case Hover:     picState = 50;     break;
    default:        picState = 10;     break;
    }

    m_iconLabel->setPixmap(QString(":/skin/images/%1_%2.svg").arg(m_iconName).arg(picState));
}

void CategoryButton::addTextShadow() {
    QGraphicsDropShadowEffect* textDropShadow = new QGraphicsDropShadowEffect;
    textDropShadow->setBlurRadius(4);
    textDropShadow->setColor(QColor(0, 0, 0, 128));
    textDropShadow->setOffset(0, 2);
    setGraphicsEffect(textDropShadow);
}
