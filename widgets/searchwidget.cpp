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
#include "global_util/util.h"

#include <QHBoxLayout>
#include <QEvent>
#include <QDebug>
#include <QKeyEvent>

SearchWidget::SearchWidget(QWidget *parent) :
    QFrame(parent)
{
    m_searchEdit = new SearchLineEdit(this);
    m_searchEdit->setAccessibleName("search-edit");
    m_searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_searchEdit);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    connect(m_searchEdit, &SearchLineEdit::textChanged, [this] {
        emit searchTextChanged(m_searchEdit->text().trimmed());
    });
}

QLineEdit *SearchWidget::edit()
{
    return m_searchEdit;
}

void SearchWidget::clearSearchContent()
{
    m_searchEdit->normalMode();
    m_searchEdit->moveFloatWidget();
}
