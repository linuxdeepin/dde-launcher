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
    void setCuted(bool isCuted);
    void addTextShadow();

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void paintEvent(QPaintEvent* event);

private:
    bool m_checked = false;
    bool m_isHighlight = false;
    bool m_isCuted = false;
};

#endif // BORDERBUTTON_H
