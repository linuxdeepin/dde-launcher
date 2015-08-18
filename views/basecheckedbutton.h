#ifndef BASECHECKEDBUTTON_H
#define BASECHECKEDBUTTON_H

#include <QPushButton>

class QEvent;

class BaseCheckedButton : public QPushButton
{
    Q_OBJECT
public:
    explicit BaseCheckedButton(QWidget *parent = 0);
    explicit BaseCheckedButton(QString text, QWidget *parent = 0);
    ~BaseCheckedButton();

signals:
    void mouseEnterToggled(bool flag);

public slots:

protected:
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
};

#endif // BASECHECKEDBUTTON_H
