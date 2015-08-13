#ifndef VIEWMODEBUTTON_H
#define VIEWMODEBUTTON_H

#include "basecheckedbutton.h"

class ViewModeButton : public BaseCheckedButton
{
    Q_OBJECT
public:
    explicit ViewModeButton(QWidget *parent = 0);
    ~ViewModeButton();

signals:

public slots:
    void setNormalIconByMode(int mode);
    void setHoverIconByMode(int mode);
};

#endif // VIEWMODEBUTTON_H
