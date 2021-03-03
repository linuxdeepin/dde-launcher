/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

SearchWidget::SearchWidget(QWidget *parent) :
    QFrame(parent)
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
    if (leftaction) {
        leftaction->setIcon(QIcon(":/icons/skin/icons/search_36px.svg"));
    }
    QAction *clearAction = m_searchEdit->findChild<QAction *>(QLatin1String("_q_qlineeditclearaction"));
    if (clearAction) {
        clearAction->setIcon(QIcon(":/icons/skin/icons/clear_36px.svg"));
    }

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

    connect(m_searchEdit, &DSearchEdit::textChanged, [this] {
        m_searchEdit->lineEdit()->setFocus();
        auto searchStr = m_searchEdit->text();
        emit searchTextChanged(searchStr.mid(0, 1) + searchStr.mid(1).replace(" ", ""));
    });

    connect(m_toggleModeBtn, &DIconButton::clicked, this, [ = ] {
        m_calcUtil->setFullScreen(false);
        DDBusSender()
        .service("com.deepin.dde.daemon.Launcher")
        .interface("com.deepin.dde.daemon.Launcher")
        .path("/com/deepin/dde/daemon/Launcher")
        .property("Fullscreen")
        .set(false);
    });

    connect(m_toggleCategoryBtn, &DIconButton::clicked, this, [ = ] {
        m_searchEdit->lineEdit()->clearFocus();
        clearSearchContent();

        emit toggleMode();

        // 点击分组按钮，切换分组模式，更新分组图标
        updateCurrentCategoryBtnIcon();
    });
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

void SearchWidget::updateSize(double scaleX, double scaleY)
{
    m_searchEdit->lineEdit()->setFixedSize(SEARCHEIT_WIDTH * scaleX, SEARCHEIT_HEIGHT * scaleY);
    double scale = (qAbs(1 - scaleX) < qAbs(1 - scaleY)) ? scaleX : scaleY;
    m_toggleCategoryBtn->setFixedSize(BTN_SIZE * scale, BTN_SIZE * scale);
    m_toggleCategoryBtn->setIconSize(QSize(ICON_SIZE * scale, ICON_SIZE * scale));
    m_toggleModeBtn->setIconSize(QSize(ICON_SIZE * scale, ICON_SIZE * scale));
    m_toggleModeBtn->setFixedSize(BTN_SIZE * scale, BTN_SIZE * scale);
}

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



