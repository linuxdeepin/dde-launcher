#ifndef CATEGORYBUTTON_H
#define CATEGORYBUTTON_H

#include "../model/appslistmodel.h"

#include <QAbstractButton>
#include <QLabel>

class CategoryButton : public QAbstractButton
{
    Q_OBJECT

public:
    enum State {
        Normal,
        Hover,
        Checked,
    };

public:
    explicit CategoryButton(const AppsListModel::AppCategory category, QWidget *parent = 0);

    AppsListModel::AppCategory category() const;

public slots:
    void setChecked(bool isChecked);

protected:
    void paintEvent(QPaintEvent *e);

private:
    void setInfoByCategory();
    void updateState(const State state);

private:
    State m_state = Checked;
    AppsListModel::AppCategory m_category;
    QString m_iconName;

    QLabel *m_iconLabel;
    QLabel *m_textLabel;
};

#endif // CATEGORYBUTTON_H
