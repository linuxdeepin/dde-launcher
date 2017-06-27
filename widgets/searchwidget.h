#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include "searchlineedit.h"

#include <QWidget>

class SearchWidget : public QFrame
{
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = 0);

    QLineEdit *edit();

public slots:
    void clearSearchContent();

signals:
    void searchTextChanged(const QString &text) const;

private:
    SearchLineEdit* m_searchEdit;
};

#endif // SEARCHWIDGET_H
