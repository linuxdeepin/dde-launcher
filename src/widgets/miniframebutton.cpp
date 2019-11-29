#include "miniframebutton.h"

#include <QDebug>
#include <QEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <DGuiApplicationHelper>
#include <QPainter>

DGUI_USE_NAMESPACE

MiniFrameButton::MiniFrameButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    setFocusPolicy(Qt::NoFocus);
    setObjectName("MiniFrameButton");
    setFlat(true);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [ = ] {
        QPalette pa = palette();
        pa.setBrush(QPalette::ButtonText, pa.brightText());
        pa.setBrush(QPalette::HighlightedText, pa.brightText());
        pa.setColor(QPalette::All, QPalette::Highlight, QColor(96, 96, 96, 160));
        setPalette(pa);
    });

    QPalette pa = palette();
    pa.setBrush(QPalette::ButtonText, pa.brightText());
    pa.setBrush(QPalette::HighlightedText, pa.brightText());
    pa.setColor(QPalette::All, QPalette::Highlight, QColor(96, 96, 96, 160));
    setPalette(pa);
}

MiniFrameButton::~MiniFrameButton()
{
}

void MiniFrameButton::enterEvent(QEvent *event)
{
    QPushButton::enterEvent(event);

    setChecked(true);

    emit entered();
}

void MiniFrameButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);

    setChecked(false);
}

bool MiniFrameButton::event(QEvent *event)
{
    if (event->type() == QEvent::ApplicationFontChange) {
        updateFont();
    }

    return QPushButton::event(event);
}

void MiniFrameButton::updateFont()
{
    QFont font = this->font();
    const int px = (qApp->font().pointSizeF() * qApp->desktop()->logicalDpiX() / 72) + 2;
    font.setPixelSize(std::max(px, 14));
    qDebug() << px;
    setFont(font);
}

void MiniFrameButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (isChecked()) {
        QPainterPath path;
        path.addRoundedRect(rect(), 4, 4);
        painter.fillPath(path, QColor(96, 96, 96, 160));
    }

    if (!icon().isNull()) {
        painter.drawPixmap(rect().center().x() - iconSize().width() / 2 + 1 , rect().center().y() - iconSize().height() / 2 + 1, icon().pixmap(iconSize()));
    }
}
