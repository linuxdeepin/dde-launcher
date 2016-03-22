#ifndef CATEGORYTITLEWIDGET_H
#define CATEGORYTITLEWIDGET_H

#include <QWidget>
#include <QLabel>

class CategoryTitleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CategoryTitleWidget(const QString &title, QWidget *parent = 0);

private:
    QLabel *m_title;
    void addTextShadow();
};

#endif // CATEGORYTITLEWIDGET_H
