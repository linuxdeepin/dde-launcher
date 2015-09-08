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

    bool isHighlight() const;
    void updateStyle();

signals:
    void rightClicked(QPoint pos);

public slots:
    void setHighlight(bool isHightlight);
    void toggleHighlight();
    void startDrag(QMouseEvent* event);

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void enterEvent(QEvent* event);

private:
    bool m_checked = false;
    bool m_isHighlight = false;
};

#endif // BORDERBUTTON_H
