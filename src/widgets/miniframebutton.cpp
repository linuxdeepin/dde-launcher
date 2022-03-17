#include "miniframebutton.h"

#include <QDebug>
#include <QEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QPainterPath>
#include <QTextOption>

DGUI_USE_NAMESPACE

MiniFrameButton::MiniFrameButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
    , m_text(text)
{
    setFocusPolicy(Qt::NoFocus);
    setObjectName("MiniFrameButton");
    setFlat(true);
    setToolTip(text);
    setToolTipDuration(1000);

    onThemeTypeChanged(DGuiApplicationHelper::instance()->themeType());
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &MiniFrameButton::onThemeTypeChanged);
}

void MiniFrameButton::onThemeTypeChanged(DGuiApplicationHelper::ColorType themeType)
{
    if (DGuiApplicationHelper::DarkType == themeType)
        m_color.setRgb(255, 255, 255, 25);
    else
        m_color.setRgb(0, 0, 0, 25);

    update();
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
        path.addRoundedRect(rect(), 8, 8);
        painter.fillPath(path, m_color);
    }

    if (!icon().isNull()) {
        QFontMetrics font(this->font());
        int textWidth = font.boundingRect(m_text).width();
        int textHeight = font.boundingRect(m_text).height();

        // 文字居中显示，左右各保持10像素边距
        int x = rect().x() + (rect().width() - (iconSize().width() + textWidth)) / 2 + 1 - 10;
        int y = rect().center().y() - iconSize().height() / 2 + 1;
        QRect pixmapRect = QRect(QPoint(x, y), iconSize());
        painter.drawPixmap(x, y, icon().pixmap(iconSize()));

        int x1 = pixmapRect.right() + 10;
        int y1 = (rect().height() - font.boundingRect(m_text).height()) / 2;

        painter.drawText(QRect(QPoint(x1, y1), QSize(rect().width() - pixmapRect.width() - 10, textHeight)), m_text);
    }
}
