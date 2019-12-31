/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     niecheng <niejiashan@163.com>
 *
 * Maintainer: niecheng <niejiashan@163.com>
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

#ifndef BLURBOXWIDGET_H
#define BLURBOXWIDGET_H

#include <QMouseEvent>
#include <QVBoxLayout>
#include <QAbstractItemDelegate>
#include <QGraphicsOpacityEffect>

#include "src/global_util/constants.h"
#include "src/global_util/calculate_util.h"
#include "src/widgets/categorytitlewidget.h"
#include "src/view/multipagesview.h"
#include "qapplication.h"
#include "maskqwidget.h"

DWIDGET_USE_NAMESPACE

class BlurBoxWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BlurBoxWidget(AppsListModel::AppCategory m_category, char *name, QWidget *parent = nullptr);

    void setMaskVisible(bool visible);
    void setMaskSize(QSize size);
    void setDataDelegate(QAbstractItemDelegate *delegate);
    void layoutAddWidget(QWidget *child);
    void layoutAddWidget(QWidget *, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
    void setCategory(AppsListModel::AppCategory category) {m_category = category;}
    MultiPagesView *getMultiPagesView();

signals:
    void maskClick(AppsListModel::AppCategory m_category, int nNext);

protected:
    void mousePressEvent(QMouseEvent *e)Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e)Q_DECL_OVERRIDE;

private:
    QPoint mousePos;
    MaskQWidget* m_bg;
    QVBoxLayout *m_vLayout ;
    MaskQWidget *m_maskLayer = nullptr;
    CalculateUtil *m_calcUtil;
    AppsListModel::AppCategory m_category;
    QString m_name;
    MultiPagesView *m_categoryMultiPagesView;
    CategoryTitleWidget *m_categoryTitle;
    QGraphicsOpacityEffect* m_titleOpacityEffect;
    QGraphicsOpacityEffect* m_pagesOpacityEffect;
};

#endif // BLURBOXWIDGET_H
