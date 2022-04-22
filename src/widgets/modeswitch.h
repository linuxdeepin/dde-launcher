#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QButtonGroup>

#include "iconbutton.h"

class ModeSwitch : public QWidget
{
    Q_OBJECT

public:
    ModeSwitch(QWidget *parent = Q_NULLPTR);
    ~ModeSwitch();

signals:
    void titleModeClicked();
    void letterModeClicked();

protected:
    void paintEvent(QPaintEvent *event);

private:
    IconButton *m_titleModeButton;
    IconButton *m_letterModeButton;
    QButtonGroup *m_buttonGrp;
};

#endif // WIDGET_H
