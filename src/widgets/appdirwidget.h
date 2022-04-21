#ifndef APPDIRWIDGET_H
#define APPDIRWIDGET_H

#include <QWidget>

class AppDirWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AppDirWidget(QWidget *parent = Q_NULLPTR);
    virtual ~AppDirWidget() {}

protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
};

#endif
