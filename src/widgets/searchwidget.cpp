// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchwidget.h"
#include "util.h"

#include <QHBoxLayout>
#include <QEvent>
#include <QAction>
#include <QDebug>
#include <QKeyEvent>
#include <DDBusSender>

#define ICON_SIZE   24
#define BTN_SIZE    40

#define SEARCHEIT_WIDTH 350
#define SEARCHEIT_HEIGHT 40

DWIDGET_USE_NAMESPACE

/**
 * @brief SearchWidget::SearchWidget 顶部搜索控件
 * @param parent
 */
SearchWidget::SearchWidget(QWidget *parent)
    : QFrame(parent)
    , m_enableUpdateMode(true)
{
    setAccessibleName("From_Search");
    m_leftSpacing = new QFrame(this);
    m_rightSpacing = new QFrame(this);

    m_calcUtil = CalculateUtil::instance();

    m_leftSpacing->setFixedWidth(0);
    m_rightSpacing->setFixedWidth(0);
    m_leftSpacing->setAccessibleName("LeftSpacing");
    m_rightSpacing->setAccessibleName("RightSpacing");

    m_toggleCategoryBtn = new DFloatingButton(this);
    updateCurrentCategoryBtnIcon();

    m_toggleCategoryBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    m_toggleCategoryBtn->setAutoExclusive(true);
    m_toggleCategoryBtn->setBackgroundRole(DPalette::Button);
    m_toggleCategoryBtn->setAccessibleName("mode-toggle-button");

    m_toggleModeBtn = new DFloatingButton(this);
    m_toggleModeBtn->setIcon(QIcon(":/icons/skin/icons/exit_fullscreen.svg"));
    m_toggleModeBtn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_toggleModeBtn->setFixedSize(QSize(BTN_SIZE, BTN_SIZE));
    m_toggleModeBtn->setAutoExclusive(true);
    m_toggleModeBtn->setBackgroundRole(DPalette::Button);
    m_toggleModeBtn->setAccessibleName("Btn-ToggleMode");

    m_searchEdit = new DSearchEdit(this);
    m_searchEdit->setAccessibleName("search-edit");

    // 添加launcher搜索框图标
    QAction *leftaction = m_searchEdit->findChild<QAction *>("_d_search_leftAction");
    if (leftaction)
        leftaction->setIcon(QIcon(":/icons/skin/icons/search_36px.svg"));

    QAction *clearAction = m_searchEdit->findChild<QAction *>(QLatin1String("_q_qlineeditclearaction"));
    if (clearAction)
        clearAction->setIcon(QIcon(":/icons/skin/icons/clear_36px.svg"));

    m_searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_searchEdit->lineEdit()->setFixedSize(SEARCHEIT_WIDTH, SEARCHEIT_HEIGHT);
    this->setMouseTracking(true);
    DStyle::setFocusRectVisible(m_searchEdit->lineEdit(), true);
    setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    mainLayout->addSpacing(30);
    mainLayout->addWidget(m_leftSpacing);
    mainLayout->addWidget(m_toggleCategoryBtn);
    mainLayout->addStretch();
    mainLayout->addWidget(m_searchEdit);
    mainLayout->addStretch();
    mainLayout->addWidget(m_toggleModeBtn);
    mainLayout->addWidget(m_rightSpacing);
    mainLayout->addSpacing(30);

    setLayout(mainLayout);

    connect(m_searchEdit, &DSearchEdit::textChanged, this, &SearchWidget::onTextChanged);
    connect(m_toggleModeBtn, &DIconButton::clicked, this, &SearchWidget::onModeClicked);
    connect(m_toggleCategoryBtn, &DIconButton::clicked, this, &SearchWidget::onToggleCategoryChanged);
}

DSearchEdit *SearchWidget::edit()
{
    return m_searchEdit;
}

DFloatingButton *SearchWidget::categoryBtn()
{
    return m_toggleCategoryBtn;
}

DFloatingButton *SearchWidget::toggleModeBtn()
{
    return m_toggleModeBtn;
}

void SearchWidget::clearSearchContent()
{
//    m_searchEdit->normalMode();
//    m_searchEdit->moveFloatWidget();
    m_searchEdit->clear();
    m_searchEdit->clearEdit();
}

void SearchWidget::onTextChanged(const QString &str)
{
    Q_UNUSED(str);

    m_searchEdit->lineEdit()->setFocus();
    auto searchStr = m_searchEdit->text();

    emit searchTextChanged(searchStr.replace(" ", ""), m_enableUpdateMode);
}

void SearchWidget::onModeClicked()
{
    m_calcUtil->setFullScreen(false);
    DDBusSender()
            .service("com.deepin.dde.daemon.Launcher")
            .interface("com.deepin.dde.daemon.Launcher")
            .path("/com/deepin/dde/daemon/Launcher")
            .property("Fullscreen")
            .set(false);
}

void SearchWidget::onToggleCategoryChanged()
{
    m_enableUpdateMode = false;
    m_searchEdit->lineEdit()->clearFocus();
    clearSearchContent();
    m_enableUpdateMode = true;

    emit toggleMode();

    // 点击分组按钮，切换分组模式，更新分组图标
    updateCurrentCategoryBtnIcon();
}

void SearchWidget::setLeftSpacing(int spacing)
{
    m_leftSpacing->setFixedWidth(spacing);
}

void SearchWidget::setRightSpacing(int spacing)
{
    m_rightSpacing->setFixedWidth(spacing);
}

void SearchWidget::showToggle()
{
    m_toggleCategoryBtn->show();
    m_toggleModeBtn->show();
}

void SearchWidget::hideToggle()
{
    m_toggleCategoryBtn->hide();
    m_toggleModeBtn->hide();
}

/**
 * @brief SearchWidget::updateSize 根据屏幕比例(相较于1920*1080分辨率的屏幕)设置控件大小
 * @param scaleX 屏幕宽度比
 * @param scaleY 屏幕高度比
 */
void SearchWidget::updateSize(double scaleX, double scaleY)
{
    m_searchEdit->lineEdit()->setFixedSize(int(SEARCHEIT_WIDTH * scaleX), int(SEARCHEIT_HEIGHT * scaleY));
    double scale = (qAbs(1 - scaleX) < qAbs(1 - scaleY)) ? scaleX : scaleY;
    m_toggleCategoryBtn->setFixedSize(int(BTN_SIZE * scale), int(BTN_SIZE * scale));
    m_toggleCategoryBtn->setIconSize(QSize(int(ICON_SIZE * scale), int(ICON_SIZE * scale)));
    m_toggleModeBtn->setIconSize(QSize(int(ICON_SIZE * scale), int(ICON_SIZE * scale)));
    m_toggleModeBtn->setFixedSize(int(BTN_SIZE * scale), int(BTN_SIZE * scale));
}

/**
 * @brief SearchWidget::updateCurrentCategoryBtnIcon 全屏模式下app列表和分类后的列表 切换按钮样式
 */
void SearchWidget::updateCurrentCategoryBtnIcon()
{
    if (m_calcUtil->displayMode() == ALL_APPS) {
        m_toggleCategoryBtn->setIcon(QIcon(":/icons/skin/icons/category_normal_22px.png"));
        m_toggleCategoryBtn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    } else {
        m_toggleCategoryBtn->setIcon(QIcon(":/icons/skin/icons/category_hover_22pxnew.svg"));
        m_toggleCategoryBtn->setIconSize(QSize(ICON_SIZE - 10, ICON_SIZE - 10));
    }
}
