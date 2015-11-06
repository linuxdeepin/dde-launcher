#include <QPainter>
#include <QDebug>
#include <QLinearGradient>

#include "gradientlabel.h"

GradientLabel::GradientLabel(QWidget *parent) :
    QLabel(parent),
    m_direction(GradientLabel::TopToBottom)
{

}

GradientLabel::Direction GradientLabel::direction() const
{
    return m_direction;
}

void GradientLabel::setDirection(const GradientLabel::Direction &direction)
{
    m_direction = direction;
}

void setText(const QString &)
{
    // do nothing !
}

void GradientLabel::paintEvent(QPaintEvent*)
{
//    QPainter painter;
//    painter.begin(this);

//    painter.fillRect(rect(), Qt::red);

//    painter.end();

    // process the pixmap
    const QPixmap *thisPix = pixmap();
    QPixmap pix(thisPix->rect().size());
    pix.fill(Qt::transparent);

    QPainter pixPainter;
    pixPainter.begin(&pix);
    pixPainter.drawPixmap(0, 0, *thisPix);

    pixPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);

    QLinearGradient gradient(pix.rect().topLeft(),
                             pix.rect().bottomLeft());

    if (m_direction == TopToBottom) {
        gradient.setColorAt(0, Qt::white);
        gradient.setColorAt(1, Qt::transparent);
    } else if (m_direction == BottomToTop) {
        gradient.setColorAt(0, Qt::transparent);
        gradient.setColorAt(1, Qt::white);
    }

    pixPainter.fillRect(pix.rect(), gradient);

    pixPainter.end();

    // draw the pixmap
    QPainter painter;
    painter.begin(this);

    painter.drawPixmap(0, 0, pix);

    painter.end();
}
