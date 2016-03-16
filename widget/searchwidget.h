#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>

#include <dsearchedit.h>

DWIDGET_USE_NAMESPACE

class SearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = 0);

public slots:
    void clearSearchContent();

signals:
    void searchTextChanged(const QString &text) const;

private:
    DSearchEdit *m_searchEdit;
};

#endif // SEARCHWIDGET_H
