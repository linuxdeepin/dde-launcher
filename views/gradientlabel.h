#ifndef GRADIENTLABEL_H
#define GRADIENTLABEL_H

#include <QLabel>

class QPaintEvent;
class GradientLabel : public QLabel
{
    Q_OBJECT
public:
    explicit GradientLabel(QWidget *parent = 0);

    enum Direction {
        TopToBottom,
        BottomToTop
    };

    void setText(const QString &);

    Direction direction() const;
    void setDirection(const Direction &direction);

private:
    Direction m_direction;

    void paintEvent(QPaintEvent* event);
};

#endif // GRADIENTLABEL_H
