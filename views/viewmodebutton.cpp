#include "viewmodebutton.h"
#include <QIcon>

ViewModeButton::ViewModeButton(QWidget *parent) : BaseCheckedButton(parent)
{
    setNormalIconByMode(1);
}

void ViewModeButton::setNormalIconByMode(int mode){
    if (mode == 1 || mode == 2){
        setStyleSheet("border-image: url(:/images/skin/images/category_50.svg);");
    }else if (mode == 0){
        setStyleSheet("border-image: url(:/images/skin/images/name_50.svg);");
    }else if (mode == 3){
        setStyleSheet("border-image: url(:/images/skin/images/install_time_50.svg);");
    }else if (mode == 4){
        setStyleSheet("border-image: url(:/images/skin/images/use_frequency_50.svg);");
    }
}

void ViewModeButton::setHoverIconByMode(int mode){
    if (mode == 1 || mode == 2){
        setStyleSheet("border-image: url(:/images/skin/images/category_100.svg);");
    }else if (mode == 0){
        setStyleSheet("border-image: url(:/images/skin/images/name_100.svg);");
    }else if (mode == 3){
        setStyleSheet("border-image: url(:/images/skin/images/install_time_100.svg);");
    }else if (mode == 4){
        setStyleSheet("border-image: url(:/images/skin/images/use_frequency_100.svg);");
    }
}

ViewModeButton::~ViewModeButton()
{

}

