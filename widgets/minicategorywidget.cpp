#include "minicategorywidget.h"

#include <QVBoxLayout>
#include <QButtonGroup>
#include <QMouseEvent>
#include <QDebug>

MiniCategoryItem::MiniCategoryItem(const QString &title, QWidget *parent) :
    QPushButton(title, parent)
{
    setCheckable(true);
}

MiniCategoryWidget::MiniCategoryWidget(QWidget *parent)
    : QWidget(parent),
      m_active(false),
      m_buttonGroup(new QButtonGroup(this))
{
    m_allApps = new MiniCategoryItem(tr("All Apps"));
    m_internet = new MiniCategoryItem(tr("Internet"));
    m_chat = new MiniCategoryItem(tr("Chat"));
    m_music = new MiniCategoryItem(tr("Music"));
    m_video = new MiniCategoryItem(tr("Video"));
    m_graphics = new MiniCategoryItem(tr("Graphics"));
    m_game = new MiniCategoryItem(tr("Game"));
    m_office = new MiniCategoryItem(tr("Office"));
    m_reading = new MiniCategoryItem(tr("Reading"));
    m_development = new MiniCategoryItem(tr("Development"));
    m_system = new MiniCategoryItem(tr("System"));

    m_items << m_allApps << m_internet << m_chat << m_music << m_video;
    m_items << m_graphics << m_game << m_office << m_reading << m_development;
    m_items << m_system;

    m_buttonGroup->addButton(m_allApps);
    m_buttonGroup->addButton(m_internet);
    m_buttonGroup->addButton(m_chat);
    m_buttonGroup->addButton(m_music);
    m_buttonGroup->addButton(m_video);
    m_buttonGroup->addButton(m_graphics);
    m_buttonGroup->addButton(m_game);
    m_buttonGroup->addButton(m_office);
    m_buttonGroup->addButton(m_reading);
    m_buttonGroup->addButton(m_development);
    m_buttonGroup->addButton(m_system);

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(m_allApps);
    buttonLayout->addWidget(m_internet);
    buttonLayout->addWidget(m_chat);
    buttonLayout->addWidget(m_music);
    buttonLayout->addWidget(m_video);
    buttonLayout->addWidget(m_graphics);
    buttonLayout->addWidget(m_game);
    buttonLayout->addWidget(m_office);
    buttonLayout->addWidget(m_reading);
    buttonLayout->addWidget(m_development);
    buttonLayout->addWidget(m_system);
    buttonLayout->setSpacing(0);
    buttonLayout->setMargin(0);

    QHBoxLayout *centralLayout = new QHBoxLayout;
    centralLayout->addStretch();
    centralLayout->addLayout(buttonLayout);
    centralLayout->addStretch();
    centralLayout->setSpacing(0);
    centralLayout->setMargin(0);

    setLayout(centralLayout);
    setObjectName("MiniCategoryWidget");
    setFocusPolicy(Qt::StrongFocus);

    m_allApps->setChecked(true);

    connect(m_allApps, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::All); });
    connect(m_internet, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Internet); });
    connect(m_chat, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Chat); });
    connect(m_music, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Music); });
    connect(m_video, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Video); });
    connect(m_graphics, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Graphics); });
    connect(m_game, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Game); });
    connect(m_office, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Office); });
    connect(m_reading, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Reading); });
    connect(m_development, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::Development); });
    connect(m_system, &QPushButton::clicked, this, [this] { emit requestCategory(AppsListModel::System); });
}

void MiniCategoryWidget::mousePressEvent(QMouseEvent *e)
{
    // ignore this event to prohibit launcher auto-hide
    Q_UNUSED(e);
}

void MiniCategoryWidget::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Down:      selectNext();           break;
    case Qt::Key_Up:        selectPrev();           break;
    case Qt::Key_Right:     emit requestRight();    break;
    default:;
    }
}

void MiniCategoryWidget::enterEvent(QEvent *e)
{
    QWidget::enterEvent(e);

    setFocus();
}

bool MiniCategoryWidget::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::FocusIn:
        m_active = true;
        emit activeChanged(m_active);
        break;
    case QEvent::FocusOut:
        m_active = false;
        emit activeChanged(m_active);
        break;
    default:;
    }

    return QWidget::event(event);
}

void MiniCategoryWidget::selectNext()
{
    int idx = 0;
    for (int i(0); i != m_items.size(); ++i)
    {
        if (m_items[i]->isChecked())
        {
            idx = i;
            break;
        }
    }

    const int next = (idx + 1) % m_items.size();
    m_items[next]->click();
}

void MiniCategoryWidget::selectPrev()
{
    int idx = 0;
    for (int i(0); i != m_items.size(); ++i)
    {
        if (m_items[i]->isChecked())
        {
            idx = i;
            break;
        }
    }

    const int prev = (idx - 1 + m_items.size()) % m_items.size();
    m_items[prev]->click();
}
