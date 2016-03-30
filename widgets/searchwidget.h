#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include "searchinputwidget.h"
#include <QWidget>

#include <dsearchedit.h>

DWIDGET_USE_NAMESPACE

class SearchWidget : public QFrame
{
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = 0);

    QLineEdit *edit();

public slots:
    void clearSearchContent();

protected:
    bool event(QEvent *e);

signals:
    void searchTextChanged(const QString &text) const;

private:
    SearchInputWidget* m_searchInputWidget;
};

#endif // SEARCHWIDGET_H
