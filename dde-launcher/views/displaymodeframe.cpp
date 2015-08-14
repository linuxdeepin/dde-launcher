#include "displaymodeframe.h"
#include "widgets/util.h"
#include "basecheckedbutton.h"
#include "baseframe.h"
#include "viewmodebutton.h"
#include "app/global.h"
#include "Logger.h"
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QTimer>

DisplayModeFrame::DisplayModeFrame(QWidget *parent) : QFrame(parent)
{
    initUI();
    initConnect();
}


void DisplayModeFrame::initUI(){
    setObjectName("DisplayModeFrame");
    setGeometry(0, 0, 160, 72);

    m_delayHideTimer = new QTimer(this);
    m_delayHideTimer->setSingleShot(true);
    m_delayHideTimer->setInterval(500);

    m_viewModeButton = new ViewModeButton(this);
    m_viewModeButton->setFixedSize(32, 32);
    m_viewModeButton->move(leftMargin, 40);

    m_buttonGroup = new QButtonGroup(this);
    m_nameButton = new BaseCheckedButton(tr("按名称"), this);
    m_buttonGroup->addButton(m_nameButton, 0);
    m_categoryIconButton = new BaseCheckedButton(tr("按分类（图标）"), this);
    m_buttonGroup->addButton(m_categoryIconButton, 1);
    m_categoryNameButton = new BaseCheckedButton(tr("按分类 （文字）"), this);
    m_buttonGroup->addButton(m_categoryNameButton, 2);
    m_installTimeButton = new BaseCheckedButton(tr("按安装时间"), this);
    m_buttonGroup->addButton(m_installTimeButton, 3);
    m_useFrequencyButton = new BaseCheckedButton(tr("按使用频率"), this);
    m_buttonGroup->addButton(m_useFrequencyButton, 4);

    m_buttonFrame = new BaseFrame(this);
    QVBoxLayout* buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(m_nameButton);
    buttonLayout->addWidget(m_categoryIconButton);
    buttonLayout->addWidget(m_categoryNameButton);
    buttonLayout->addWidget(m_installTimeButton);
    buttonLayout->addWidget(m_useFrequencyButton);
    buttonLayout->setSpacing(10);
    buttonLayout->setContentsMargins(0, 20, 0, 0);
    m_buttonFrame->setLayout(buttonLayout);

    m_buttonFrame->move(m_viewModeButton->x(), m_viewModeButton->y() + m_viewModeButton->height());
    m_buttonFrame->hide();

    setStyleSheet(getQssFromFile(":/qss/skin/qss/buttons.qss"));
}


void DisplayModeFrame::initConnect(){
    connect(m_viewModeButton, SIGNAL(mouseEnterToggled(bool)), this, SLOT(toggleButtonFrameByViewButton(bool)));
    connect(m_buttonFrame, SIGNAL(mouseEnterToggled(bool)), this, SLOT(toggleButtonFrameBySelf(bool)));
    connect(m_delayHideTimer, SIGNAL(timeout()), this, SLOT(hideButtonFrame()));
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(setViewMode(int)));
}

void DisplayModeFrame::toggleButtonFrameByViewButton(bool flag){
    m_viewModeButton->setHoverIconByMode(m_viewMode);
    if (flag && !m_buttonFrame->isVisible()){
        showButtonFrame();
    }else{
        m_delayHideTimer->start();
    }
}

void DisplayModeFrame::toggleButtonFrameBySelf(bool flag){
    if (!flag){
       m_delayHideTimer->start();
    }else{
        m_delayHideTimer->stop();
    }
}

void DisplayModeFrame::showButtonFrame(){
    setGeometry(0, 0, 160, 200);
    m_buttonFrame->show();
    emit visibleChanged(true);
}

void DisplayModeFrame::hideButtonFrame(){
    setGeometry(0, 0, 160, 72);
    m_buttonFrame->hide();
    m_viewModeButton->setNormalIconByMode(m_viewMode);
    emit visibleChanged(false);
}

int DisplayModeFrame::getCategoryDisplayMode(){
    return m_categoryDisplayMode;
}

void DisplayModeFrame::setCategoryDisplayMode(int mode){
    m_categoryDisplayMode = mode;
    emit categoryModeChanged(mode);
}

int DisplayModeFrame::getSortMode(){
    return m_sortMode;
}

void DisplayModeFrame::setSortMode(int mode){

    m_sortMode = mode;
    emit sortModeChanged(mode);
}

void DisplayModeFrame::setViewMode(int mode){
    m_viewMode = mode;
    m_viewModeButton->setNormalIconByMode(m_viewMode);
    if (mode == 0 || mode == 3 || mode == 4){
        setSortMode(mode);
    }else if (mode == 1 || mode ==2){
        setCategoryDisplayMode(mode);
    }

    hideButtonFrame();
}

DisplayModeFrame::~DisplayModeFrame()
{

}

