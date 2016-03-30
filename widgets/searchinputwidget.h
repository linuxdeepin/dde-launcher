#ifndef SEARCHINPUTWIDGET_H
#define SEARCHINPUTWIDGET_H

#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>

class SearchLineEdit: public QLineEdit{
    Q_OBJECT
public:
    SearchLineEdit(QWidget* parent=0);
    ~SearchLineEdit();
signals:
    void focusChanged(bool isFocus);
protected:
    void focusInEvent(QFocusEvent* e);
    void focusOutEvent(QFocusEvent* e);
//    void mousePressEvent(QMouseEvent* e);
//    void mouseReleaseEvent(QMouseEvent* e);
};

class SearchInputWidget:public QFrame {
    Q_OBJECT
public:
    SearchInputWidget(QFrame* parent=0);
    ~SearchInputWidget();
    QLineEdit* lineEdit();
signals:
    void textChanged(const QString &text) const;
public slots:
    void handleInputFocusChanged(bool isFocus);
    const QString text();
    void clear();
private:
    QLabel* m_searchIcon;
    SearchLineEdit* m_searchLineEdit;
    QHBoxLayout* m_Layout;
};
#endif // SEARCHINPUTWIDGET_H
