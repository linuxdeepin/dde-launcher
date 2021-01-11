
#include "accessibledefine.h"

#include <DIconButton>
#include <DSwitchButton>
#include <DPushButton>

#include <dboxwidget.h>

#include <QScrollBar>
#include <QLineEdit>
#include <QMenu>

#include "./widgets/avatar.h"
#include "./widgets/categorybutton.h"
#include "./widgets/categorytitlewidget.h"
#include "./widgets/datetimewidget.h"
#include "./widgets/gradientlabel.h"
#include "./widgets/hseparator.h"
#include "./widgets/minicategorywidget.h"
#include "./widgets/miniframebutton.h"
#include "./widgets/miniframenavigation.h"
#include "./widgets/miniframerightbar.h"
#include "./widgets/miniframeswitchbtn.h"
#include "./widgets/navigationwidget.h"
#include "./widgets/roundedbutton.h"
#include "./widgets/searchlineedit.h"
#include "./widgets/applistarea.h"

#include "./view/appgridview.h"
#include "./view/applistview.h"

#include "fullscreenframe.h"
#include "windowedframe.h"


DWIDGET_USE_NAMESPACE

// 添加accessible

SET_BUTTON_ACCESSIBLE(DIconButton, m_w->objectName())
SET_BUTTON_ACCESSIBLE(DSwitchButton, m_w->text())
SET_FORM_ACCESSIBLE(AppGridView, "AppGridView")
SET_BUTTON_ACCESSIBLE(MiniFrameButton, m_w->accessibleName())
SET_FORM_ACCESSIBLE(QMenu, "Menu")
SET_FORM_ACCESSIBLE(AppListArea, "AppListArea")
SET_FORM_ACCESSIBLE(FullScreenFrame, "FullScreenFrame")
SET_FORM_ACCESSIBLE(NavigationWidget, "NavigationWidget")
SET_LABEL_ACCESSIBLE(BoxFrame, "LauncherFrame")
SET_FORM_ACCESSIBLE(SearchWidget, "SearchWidget")
SET_EDITABLE_ACCESSIBLE(QLineEdit, "LineEdit")
SET_FORM_ACCESSIBLE(DatetimeWidget, "DatetimeWidget")
SET_BUTTON_ACCESSIBLE(Avatar, "Avatar")
SET_BUTTON_ACCESSIBLE(MiniFrameSwitchBtn, "MiniFrameSwitchBtn")
SET_FORM_ACCESSIBLE(HSeparator, "HSeparator")
SET_FORM_ACCESSIBLE(MiniFrameRightBar, "MiniFrameRightBar")
SET_FORM_ACCESSIBLE(WindowedFrame, "WindowedFrame")
SET_BUTTON_ACCESSIBLE(RoundedButton, m_w->objectName())
SET_FORM_ACCESSIBLE(QWidget, "widget")
SET_FORM_ACCESSIBLE(QFrame, "frame")
SET_SLIDER_ACCESSIBLE(QScrollBar, "scrollerbar")
SET_LABEL_ACCESSIBLE(GradientLabel, "gradientlabel")
SET_FORM_ACCESSIBLE(QPushButton, "QPushButton")
SET_FORM_ACCESSIBLE(QToolButton, "QToolButton")
SET_FORM_ACCESSIBLE(QLabel, "QLabel")
SET_FORM_ACCESSIBLE(MaskQWidget, "MaskQWidget")
SET_FORM_ACCESSIBLE(DHBoxWidget, "DHBoxWidget")
SET_FORM_ACCESSIBLE(DBoxWidget, "DBoxWidget")


QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    // 自动化标记确定不需要的控件，方可加入忽略列表,QLabel目前DTK没有修改完成，导致部分QLabel没有设置AccessName,导致过不了Q_ASSERT_X
    // TO_DO dtk完善DSearchEdit设置AccessName内容
    const static QStringList ignoreLst = {"QLineEditIconButton","QLabel"};

    QAccessibleInterface *interface = nullptr;
    USE_ACCESSIBLE(classname, AppGridView);
    USE_ACCESSIBLE(classname, AppListArea);
    USE_ACCESSIBLE(classname, FullScreenFrame);
    USE_ACCESSIBLE(classname, MiniFrameButton);
    USE_ACCESSIBLE(classname, BoxFrame);
    USE_ACCESSIBLE(classname, SearchWidget);
    USE_ACCESSIBLE(classname, AppGridView);
    USE_ACCESSIBLE(classname, QLineEdit);
    USE_ACCESSIBLE(classname, DatetimeWidget);
    USE_ACCESSIBLE(classname, Avatar);
    USE_ACCESSIBLE(classname, WindowedFrame);
    USE_ACCESSIBLE(classname, HSeparator);
    USE_ACCESSIBLE(classname, MiniFrameRightBar);
    USE_ACCESSIBLE(classname, MiniFrameSwitchBtn);
    USE_ACCESSIBLE(classname, RoundedButton);
    USE_ACCESSIBLE_BY_OBJECTNAME(QString(classname).replace("Dtk::Widget::", ""), DIconButton, object->objectName());
    USE_ACCESSIBLE_BY_OBJECTNAME(QString(classname).replace("Dtk::Widget::", ""), DSwitchButton, object->objectName());
    USE_ACCESSIBLE(classname, QWidget);
    USE_ACCESSIBLE(classname, QFrame);
    USE_ACCESSIBLE(classname, QScrollBar);
    USE_ACCESSIBLE(classname, GradientLabel);
    USE_ACCESSIBLE(classname, QPushButton);
    USE_ACCESSIBLE(classname, QToolButton);
    USE_ACCESSIBLE(classname, MaskQWidget);
    USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DHBoxWidget);
    USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DBoxWidget);

    if (!interface && object->inherits("QWidget") && !ignoreLst.contains(classname)) {
        QWidget *w = static_cast<QWidget *>(object);
        // 如果你崩溃到这里，说明你修改的代码未兼顾到accessible功能，请修改通过后再提交
        if (w->accessibleName().isEmpty())
            Q_ASSERT_X(!w || !w->accessibleName().isEmpty(), "accessibleFactory()", QString("Class: " + classname + " cannot access").toLatin1().data());
    }

    return interface;
}
