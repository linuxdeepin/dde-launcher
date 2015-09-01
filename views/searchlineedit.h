#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QFrame>
class QLabel;
class DLineEdit;
class QResizeEvent;

class SearchLineEdit : public QFrame
{
    Q_OBJECT
public:
    explicit SearchLineEdit(QWidget *parent = 0);
    void initUI();
    void initConnect();

protected:
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
