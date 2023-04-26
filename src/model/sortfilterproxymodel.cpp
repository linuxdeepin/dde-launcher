// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sortfilterproxymodel.h"
#include "appslistmodel.h"
#include "iteminfo.h"
#include "languagetranformation.h"

#include <DPinyin>
#include <QDebug>

DCORE_USE_NAMESPACE

SortFilterProxyModel::SortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel (parent)
    , m_languageSwitch(LanguageTransformation::instance())
{
}

static bool matchPinyin(const QString &text, const QString &appName)
{
    for (const QString &firstLetter : firstLetters(appName)) {
        if (firstLetter.contains(text, Qt::CaseInsensitive))
            return true;
    }

    for (const QString &py : pinyin(appName, TS_NoneTone)) {
        if (py.contains(text, Qt::CaseInsensitive))
            return true;
    }

    return false;
}

bool SortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex modelIndex = this->sourceModel()->index(sourceRow, 0, sourceParent);
    const ItemInfo_v1 &info = modelIndex.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();

    QString searchedText = filterRegExp().pattern();

    return info.m_desktop.contains(searchedText, Qt::CaseInsensitive) ||
           info.m_name.contains(searchedText, Qt::CaseInsensitive) ||
           info.m_key.contains(searchedText, Qt::CaseInsensitive) ||
           matchPinyin(searchedText, info.m_name);
}
