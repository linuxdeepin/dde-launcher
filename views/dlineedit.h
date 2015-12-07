#ifndef DLINEEDIT_H
#define DLINEEDIT_H

#include <QLineEdit>
#include <QKeyEvent>

class DLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit DLineEdit(QWidget *parent = 0);

signals:

public slots:

protected:
    void keyPressEvent(QKeyEvent* event);

};

#endif // DLINEEDIT_H
