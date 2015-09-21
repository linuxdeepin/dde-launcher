#ifndef CATEGORYITEM_H
#define CATEGORYITEM_H

#include <QFrame>

class QLabel;

class CategoryItem : public QFrame
{
    Q_OBJECT
public:
    explicit CategoryItem(QString text, QWidget *parent = 0);
    ~CategoryItem();

    void initUI(QString text);

signals:

public slots:
    void addTextShadow();

private:
    QLabel* m_titleLabel;
};

#endif // CATEGORYITEM_H
