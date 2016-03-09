#ifndef CATEGORYTITLEWIDGET_H
#define CATEGORYTITLEWIDGET_H

#include <QWidget>
#include <QLabel>

class CategoryTitleWidget : public QWidget
{
    Q_OBJECT

public:
    CategoryTitleWidget(const QString &title, QWidget *parent = 0);

private:
    QLabel *m_title;
};

#endif // CATEGORYTITLEWIDGET_H
