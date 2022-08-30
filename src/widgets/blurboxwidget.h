// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLURBOXWIDGET_H
#define BLURBOXWIDGET_H

#include <QMouseEvent>
#include <QVBoxLayout>
#include <QAbstractItemDelegate>
#include <QSharedPointer>

#include <DBlurEffectWidget>

#include "constants.h"
#include "calculate_util.h"
#include "categorytitlewidget.h"
#include "multipagesview.h"
#include "qapplication.h"
#include "maskqwidget.h"

DWIDGET_USE_NAMESPACE

enum OperationType {
    otNone  = 0,
    otLeft  = 1,
    otRight = 2
};

class BlurBoxWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BlurBoxWidget(AppsListModel::AppCategory m_category, char *name, QWidget *parent = nullptr);
    ~BlurBoxWidget() override;

    void setMaskVisible(bool visible);
    void setMaskSize(QSize size);
    void setDataDelegate(QAbstractItemDelegate *delegate);
    void layoutAddWidget(QWidget *, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
    void setCategory(AppsListModel::AppCategory category) {m_category = category;}
    AppsListModel::AppCategory category() { return  m_category;}
    MultiPagesView *getMultiPagesView();
    void updateBackBlurPos(QPoint p);
    static void updateBackgroundImage(const QPixmap & img);
    void setBlurBgVisible(bool visible);
    void setOperationType(OperationType operType) { m_operationType = operType;}
    void setFixedSize(const QSize &size);
    CategoryTitleWidget *categoryTitle() { return m_categoryTitle;}

signals:
    void maskClick(AppsListModel::AppCategory appCategory);

protected:
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

private:
    QPoint mousePos;
    QVBoxLayout *m_vLayout ;
    MaskQWidget *m_maskLayer = nullptr;
    CalculateUtil *m_calcUtil;
    AppsListModel::AppCategory m_category;
    QString m_name;
    MultiPagesView *m_categoryMultiPagesView;
    CategoryTitleWidget *m_categoryTitle;
    static QSharedPointer<DBlurEffectGroup> m_blurGroup;
    DBlurEffectWidget *m_blurBackground;
    MaskQWidget* m_bg;
    OperationType m_operationType = otNone;
};

#endif // BLURBOXWIDGET_H
