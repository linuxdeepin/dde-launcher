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

public slots:
    void normalMode();

private slots:
    void editMode();
    void onTextChanged();

protected:
    bool event(QEvent *e);

private:
    DImageButton *m_icon;
    DImageButton *m_clear;
    QLabel *m_placeholderText;
    QWidget *m_floatWidget;
#ifndef ARCH_MIPSEL
    QPropertyAnimation *m_floatAni;
#endif
};

#endif // SEARCHLINEEDIT_H
