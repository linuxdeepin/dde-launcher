#ifndef BORDERBUTTON_H
#define BORDERBUTTON_H

#include <QPushButton>
#include <QPoint>
class QMouseEvent;
class QWidget;

class BorderButton : public QPushButton
{
    Q_OBJECT
public:
    explicit BorderButton(QWidget *parent = 0);
    ~BorderButton();

signals:
    void rightClicked(QPoint pos);

public slots:


protected:
    void mousePressEvent(QMouseEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);

private:
    bool m_checked = false;
};

#endif // BORDERBUTTON_H
