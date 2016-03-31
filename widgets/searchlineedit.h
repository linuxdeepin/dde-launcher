#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QLineEdit>
#include <QLabel>
#include <QSpacerItem>
#include <QPropertyAnimation>

#include <dimagebutton.h>

DWIDGET_USE_NAMESPACE

class SearchLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit SearchLineEdit(QWidget *parent = nullptr);

    bool event(QEvent *e);

private:
    void normalMode();
    void editMode();

private:
    DImageButton *m_icon;
    QLabel *m_placeholderText;
    QWidget *m_floatWidget;
    QPropertyAnimation *m_floatAni;
};

#endif // SEARCHLINEEDIT_H
