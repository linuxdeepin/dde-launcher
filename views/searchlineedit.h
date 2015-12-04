#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QFrame>
#include <QCommonStyle>
class QLabel;
class DLineEdit;
class QResizeEvent;


class LineEditStyle : public QCommonStyle
{
public:
    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;
};

class SearchLineEdit : public QFrame
{
    Q_OBJECT

public:
    explicit SearchLineEdit(QWidget *parent = 0);
    void initUI();
    void initConnect();
    QString getText();

protected:
    void keyPressEvent(QKeyEvent* event);
    void resizeEvent(QResizeEvent *event);

signals:
    void textChanged(const QString& text);

public slots:
    void handleTextChanged(const QString& text);
    void setText(const QString& text);
    void setSearchFocus();

private:
    QLabel* m_iconLabel;
    DLineEdit* m_lineEdit;
};

#endif // SEARCHLINEEDIT_H
