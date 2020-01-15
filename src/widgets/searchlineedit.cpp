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

#include "searchlineedit.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QEvent>
#include <QTimer>
#include <QResizeEvent>
#include <QGuiApplication>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

SearchLineEdit::SearchLineEdit(QWidget *parent) :
    QLineEdit(parent)
    , m_icon(new DIconButton(this))
    , m_clear(new DIconButton(this))
{
    setTextMargins(20, 0, 15, 0);
    m_icon->setIconSize(QSize(16, 16));
    m_icon->setFixedSize(16, 16);
    m_icon->setFlat(true);

    m_clear->setIconSize(QSize(16, 16));
    m_clear->setFixedSize(16, 16);
    m_clear->setVisible(false);
    m_clear->setFlat(true);

    themeChanged();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [ = ] {
        themeChanged();
    });

    m_placeholderText = new QLabel(tr("Search"));
    QFontMetrics fm(m_placeholderText->font());
    m_placeholderText->setFixedWidth(fm.width(m_placeholderText->text()) + 10);
    m_placeholderText->setForegroundRole(QPalette::BrightText);
    m_floatWidget = new QWidget(this);

    m_editStyle = new SearchLineeditStyle(style());

    QHBoxLayout *floatLayout = new QHBoxLayout;
    floatLayout->addWidget(m_icon);
    floatLayout->setAlignment(m_icon, Qt::AlignVCenter);
    floatLayout->addStretch();
    floatLayout->addWidget(m_placeholderText);
    floatLayout->setAlignment(m_placeholderText, Qt::AlignVCenter);
    floatLayout->setSpacing(0);
    floatLayout->setMargin(0);

    //m_floatWidget->setFixedHeight(30);
    m_floatWidget->setFixedWidth(m_icon->width() + m_placeholderText->width() + 5);
    m_floatWidget->setLayout(floatLayout);

    QHBoxLayout *centralLayout = new QHBoxLayout;
    centralLayout->addStretch();
    centralLayout->addWidget(m_clear);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 5, 0);

    setLayout(centralLayout);
    setContextMenuPolicy(Qt::NoContextMenu);
    setFocusPolicy(Qt::ClickFocus);
    //setFixedHeight(30);
    setObjectName("SearchEdit");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setStyle(m_editStyle);

    m_icon->setFocusPolicy(Qt::NoFocus);
    m_clear->setFocusPolicy(Qt::NoFocus);

    connect(this, &SearchLineEdit::textChanged, this, &SearchLineEdit::onTextChanged);
    connect(m_clear, &DIconButton::clicked, this, &SearchLineEdit::normalMode);

    connect(qApp, &QGuiApplication::fontChanged, this, [ = ](const QFont & font) {
        m_placeholderText->setFont(font);
        QFontMetrics fm(font);
        m_placeholderText->setFixedWidth(fm.width(m_placeholderText->text()) + 10);
        m_floatWidget->setFixedWidth(m_icon->width() + m_placeholderText->width() + 5);
    });

#ifndef ARCH_MIPSEL
    m_floatAni = new QPropertyAnimation(m_floatWidget, "pos", this);
    m_floatAni->setDuration(260);
    m_floatAni->setEasingCurve(QEasingCurve::OutQuad);

    connect(m_floatAni, &QPropertyAnimation::finished, this, static_cast<void (SearchLineEdit::*)()>(&SearchLineEdit::update), Qt::QueuedConnection);
    connect(m_floatAni, &QPropertyAnimation::finished, this, &SearchLineEdit::moveFloatWidget, Qt::QueuedConnection);
#endif

    normalMode();
}

bool SearchLineEdit::event(QEvent *e)
{
    switch (e->type()) {
#ifdef ARCH_MIPSEL
    case QEvent::InputMethodQuery: // for loongson, there's no FocusIn event when the widget gets focus.
#endif
    case QEvent::FocusIn:       editMode();         break;
    case QEvent::KeyPress: {
        if (QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e)) {
            if (keyEvent->matches(QKeySequence::Undo)) {
                QString oldText = this->text();
                undo();
                if (!oldText.isEmpty() && oldText == this->text()) {
                    this->clear();
                }
                return true;
            }
        }
    }
    break;
    //    case QEvent::FocusOut:      normalMode();       break;
    default:;
    }

    return QLineEdit::event(e);
}

void SearchLineEdit::resizeEvent(QResizeEvent *e)
{
    QLineEdit::resizeEvent(e);

    QTimer::singleShot(1, this, &SearchLineEdit::moveFloatWidget);

    m_floatWidget->setFixedHeight(30* qApp->devicePixelRatio());
    setFixedHeight(30* qApp->devicePixelRatio());
}

void SearchLineEdit::normalMode()
{
    // clear text when back to normal mode
    clear();
    clearFocus();

    m_placeholderText->show();
    m_clear->setVisible(false);
    m_editStyle->hideCursor = true;

#ifndef ARCH_MIPSEL
    m_floatAni->setEndValue(QPoint(rect().center().x() - m_floatWidget->rect().center().x(),0));
    if (m_floatAni->state() == QPropertyAnimation::Running)
        return;
    m_floatAni->setStartValue(m_floatWidget->pos());
    m_floatAni->start();
#else
    m_floatWidget->move(rect().center() - m_floatWidget->rect().center());
#endif
}

void SearchLineEdit::editMode()
{
    m_placeholderText->hide();

#ifndef ARCH_MIPSEL
    m_floatAni->stop();
    m_floatAni->setStartValue(m_floatWidget->pos());
    m_floatAni->setEndValue(QPoint(5, 0));
    m_floatAni->start();
    m_clear->setVisible(true);

    m_editStyle->hideCursor = true;
    QTimer::singleShot(m_floatAni->duration(), this, [ = ] {
        m_editStyle->hideCursor = false;
    });
#else
    m_floatWidget->move(QPoint(5, 0));
#endif
}

void SearchLineEdit::onTextChanged()
{
    if (!this->text().isEmpty()) {
        this->setFocus();
    }
}

void SearchLineEdit::moveFloatWidget()
{
#ifndef ARCH_MIPSEL
    if (m_floatAni->endValue().toPoint() == QPoint(5, 0))
        return;

    m_floatAni->stop();
#else
    if (!m_placeholderText->isVisible() && m_floatWidget->pos() != QPoint(5, 0))
        return;
#endif

    m_floatWidget->move(rect().center() - m_floatWidget->rect().center());
}

void SearchLineEdit::themeChanged()
{
    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        m_icon->setIcon(QIcon::fromTheme(":/skin/images/search-dark.svg"));
        m_clear->setIcon(QIcon::fromTheme(":/icons/skin/icons/input_clear_normal-dark.svg"));
    } else {
        m_icon->setIcon(QIcon::fromTheme(":/skin/images/search.svg"));
        m_clear->setIcon(QIcon::fromTheme(":/icons/skin/icons/input_clear_normal.svg"));
    }

    QPalette pa = palette();
    pa.setBrush(QPalette::Text, pa.brightText());
    pa.setBrush(QPalette::Foreground, pa.brightText());
    //pa.setColor(QPalette::Button, Qt::transparent); // 背景
    pa.setColor(QPalette::Highlight, Qt::transparent); // 激活后的边框
    setPalette(pa);
}

SearchLineeditStyle::SearchLineeditStyle(QStyle *style)
    : QProxyStyle(style)
    , hideCursor(false)
{

}

int SearchLineeditStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    if (metric == QStyle::PM_TextCursorWidth && hideCursor)
        return 0;

    return QProxyStyle::pixelMetric(metric, option, widget);
}

