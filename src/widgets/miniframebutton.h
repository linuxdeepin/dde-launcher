#ifndef MINIFRAMEBUTTON_H
#define MINIFRAMEBUTTON_H

#include <DGuiApplicationHelper>

#include <QPushButton>

DGUI_USE_NAMESPACE

class MiniFrameButton : public QPushButton
{
    Q_OBJECT

public:
    explicit MiniFrameButton(const QString &text, QWidget *parent = nullptr);

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onThemeTypeChanged(DGuiApplicationHelper::ColorType themeType);

private:
    void updateFont();

private:
    QColor m_color;
    QString m_text;
};

#endif
