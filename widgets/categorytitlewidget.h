#ifndef CATEGORYTITLEWIDGET_H
#define CATEGORYTITLEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include "global_util/calculate_util.h"

class CategoryTitleWidget : public QFrame
{
    Q_OBJECT

public:
    explicit CategoryTitleWidget(const QString &title, QWidget *parent = 0);

    QLabel *textLabel();

public slots:
    void setTextVisible(const bool visible);
    void setText(const QString &title);
protected:
    void mouseReleaseEvent(QMouseEvent *e);
private:
    CalculateUtil *m_calcUtil;
    QLabel *m_title;
    void addTextShadow();
};

#endif // CATEGORYTITLEWIDGET_H
