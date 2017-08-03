#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include <QWidget>
#include <QListView>

class HistoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryWidget(QWidget *parent = 0);

private:
    QListView *m_historyView;
};

#endif // HISTORYWIDGET_H
