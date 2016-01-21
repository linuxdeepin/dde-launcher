#include "launcherframe.h"
#include "widgets/util.h"
#include "displaymodeframe.h"
#include "apptablewidget.h"
#include "categoryframe.h"
#include "navigationbar.h"
#include "app/global.h"
#include "app/xcb_misc.h"
#include "Logger.h"
#include "searchlineedit.h"
#include "categorytablewidget.h"
#include "background/backgroundlabel.h"
#include "baseframe.h"
#include "dbusinterface/displayinterface.h"
#include "dbusinterface/dock_interface.h"
#include "dbusinterface/dbuscontrolcenter.h"
#include "app/launcherapp.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QStackedLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QPushButton>
#include <QApplication>
#include <QCloseEvent>
#include <QDBusConnection>
#include <QGraphicsDropShadowEffect>
#include <views/gradientlabel.h>


int LauncherFrame::AppItemTopSpacing = 14;
int LauncherFrame::AppItemIconNameSpacing = 8;
int LauncherFrame::AppItemMargin = 10;
int LauncherFrame::IconSize = 64;
int LauncherFrame::TextHeight = 34;
int LauncherFrame::GridSpacing = 24;
int LauncherFrame::GridSize = 160;
int LauncherFrame::BorderSize = 144;
int LauncherFrame::BorderWidth = 100;
int LauncherFrame::BorderHeight = 100;


LauncherFrame::LauncherFrame(QWidget *parent) : QFrame(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_InputMethodEnabled);
    setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);

    QRect primaryRect = QRect(dbusController->getDisplayInterface()->primaryRect());
    m_primaryPos = QPoint(primaryRect.x(), primaryRect.y());
    move(m_primaryPos);
    setFixedSize(primaryRect.width(), primaryRect.height());


    setObjectName("LauncherFrame");
    computerGrid(160, 60, LauncherFrame::GridSpacing, LauncherFrame::GridSize);
    appItemManager->init();
    initUI();
    initConnect();
    setStyleSheet(getQssFromFile(":/qss/skin/qss/main.qss"));
    qDebug() << geometry();

    m_searchTimer = new QTimer;
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(100);
    connect(m_searchTimer, SIGNAL(timeout()), this, SLOT(search()));
}

void LauncherFrame::initUI(){
    qDebug() << "initUI";
    m_backgroundLabel = new BackgroundLabel(true, this);
    m_clearCheckedButton = new QPushButton(this);
    m_clearCheckedButton->setCheckable(true);
    m_clearCheckedButton->hide();

    m_categoryFrame = new CategoryFrame(this);
    m_categoryFrame->initUI(m_leftMargin, m_rightMargin, m_column, m_itemWidth, m_gridWidth, m_gridHeight);

    QFrame* appBox = new QFrame;
    appBox->setObjectName("AppBox");
    m_appTableWidget = new AppTableWidget(this);
    m_appTableWidget->setGridParameter(m_column, m_gridWidth, m_gridHeight, m_itemWidth);

    QHBoxLayout* appLayout = new QHBoxLayout(appBox);
    appLayout->addWidget(m_appTableWidget);
    appLayout->setContentsMargins(m_leftMargin, m_topMargin, m_rightMargin, m_bottomMargin);
    appBox->setLayout(appLayout);

    m_layout = new QStackedLayout(this);
    m_layout->addWidget(m_categoryFrame);
    m_layout->addWidget(appBox);
    m_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout);

    int mode = dbusController->getSortMethod();
    qDebug() <<  "initUI" << mode;
    if (mode == 0 || mode == 2 || mode == 3){
        m_layout->setCurrentIndex(1);
    }else{
        m_layout->setCurrentIndex(0);
    }
    m_displayModeFrame = new DisplayModeFrame(this);

    m_searchLineEdit = new SearchLineEdit(this);

    m_topGradient = new GradientLabel(this);
    m_bottomGradient = new GradientLabel(this);
    m_bottomGradient->setDirection(GradientLabel::BottomToTop);

}

void LauncherFrame::computerGrid(int minimumLeftMargin, int minimumTopMargin, int miniSpacing, int itemWidth){
    Q_UNUSED(itemWidth)
    int desktopWidth = width();
    int desktopHeight = height();

    if (desktopWidth <= 700){
        m_column = 4;
    }else if (desktopWidth <= 800){
        m_column = 5;
    }else{
        m_column = 7;
    }

    if (desktopHeight <= 400){
        m_row = 2;
    }else if (desktopHeight <= 700){
        m_row = 3;
    }else{
        m_row = 4;
    }

    m_spacing = miniSpacing;

    m_itemWidth =  (desktopWidth - minimumLeftMargin * 2 - m_spacing * m_column) / m_column;
    m_gridHeight =  (desktopHeight - minimumTopMargin * 2 - m_spacing * m_row) / m_row;
    m_gridWidth = m_spacing + m_itemWidth;

    m_leftMargin = (desktopWidth - m_column * m_gridWidth)/ 2;
    m_rightMargin = desktopWidth - m_leftMargin - m_column * m_gridWidth;

    m_topMargin = (desktopHeight - m_row * m_gridHeight) / 2;
    m_bottomMargin = desktopHeight - m_row * m_gridHeight - m_topMargin;

//    m_itemWidth = itemWidth;
//    m_column = (desktopWidth - minimumLeftMargin * 2) / (itemWidth + miniSpacing);
//    m_spacing = (desktopWidth  - minimumLeftMargin * 2) / m_column - itemWidth;
//    m_gridWidth = m_spacing + itemWidth;
//    m_leftMargin = (desktopWidth - m_column * m_gridWidth)/ 2;
//    m_rightMargin = desktopWidth - m_leftMargin - m_column * m_gridWidth;

//    m_row = (desktopHeight - minimumTopMargin) / m_gridWidth;
//    m_topMargin = (desktopHeight - m_row * m_gridWidth) / 2;
//    m_bottomMargin = desktopHeight - m_row * m_gridWidth - m_topMargin;


    LauncherFrame::AppItemTopSpacing = 4;
    LauncherFrame::AppItemIconNameSpacing = 4;
    LauncherFrame::AppItemMargin = 4;

    if (m_itemWidth > 2 * 64){
        LauncherFrame::IconSize = 64;
    }else{
        LauncherFrame::IconSize = 48;
    }

    int appItemDefaultHeight = LauncherFrame::AppItemTopSpacing + LauncherFrame::AppItemIconNameSpacing + LauncherFrame::IconSize + LauncherFrame::TextHeight;
    LauncherFrame::BorderHeight = m_gridHeight - m_spacing;
    if (LauncherFrame::BorderHeight < appItemDefaultHeight){
        qDebug() << "less than "<< appItemDefaultHeight;
        if (LauncherFrame::BorderHeight > (LauncherFrame::IconSize + LauncherFrame::TextHeight)){
            LauncherFrame::AppItemTopSpacing = (LauncherFrame::BorderHeight - LauncherFrame::IconSize - LauncherFrame::TextHeight) / 2;
            LauncherFrame::AppItemIconNameSpacing = LauncherFrame::AppItemTopSpacing;
            LauncherFrame::AppItemMargin = 0;
        }else{
            LauncherFrame::BorderHeight = LauncherFrame::IconSize + LauncherFrame::TextHeight;
            LauncherFrame::AppItemTopSpacing = 0;
            LauncherFrame::AppItemIconNameSpacing = 0;
            LauncherFrame::AppItemMargin = 0;
        }
    }else{
        LauncherFrame::BorderHeight = m_gridHeight - m_spacing;
        int spacing = (LauncherFrame::BorderHeight - LauncherFrame::IconSize - LauncherFrame::TextHeight) / 4;
        LauncherFrame::AppItemTopSpacing = spacing;
        LauncherFrame::AppItemIconNameSpacing = 8;
        LauncherFrame::AppItemMargin = 4;

        qDebug() << "more than "<<  appItemDefaultHeight << spacing;

    }
    LauncherFrame::BorderWidth = m_itemWidth;

    if (desktopWidth > 1440){
        LauncherFrame::BorderHeight = 144;
        LauncherFrame::BorderWidth = 144;
    }else if (desktopWidth == 1440){
        LauncherFrame::BorderHeight = 120;
        LauncherFrame::BorderWidth = 120;
    }

    qDebug() << m_column << m_itemWidth  << m_gridWidth << m_gridHeight << m_spacing << m_leftMargin << m_rightMargin;
    qDebug() << LauncherFrame::BorderWidth << LauncherFrame::BorderHeight;
    qDebug() << m_row << m_topMargin << m_bottomMargin;
}


void LauncherFrame::initConnect(){
    connect(m_displayModeFrame, SIGNAL(visibleChanged(bool)), this, SLOT(toggleDisableNavgationBar(bool)));
    connect(m_displayModeFrame, SIGNAL(sortModeChanged(int)), this, SLOT(showSortedMode(int)));
    connect(m_displayModeFrame, SIGNAL(categoryModeChanged(int)), this, SLOT(showCategoryMode(int)));
    connect(m_searchLineEdit, SIGNAL(textChanged(QString)), this, SLOT(handleSearch(QString)));
    connect(signalManager, SIGNAL(startSearched(QString)), m_searchLineEdit, SLOT(setText(QString)));
    connect(signalManager, SIGNAL(searchItemInfoListChanged(QList<ItemInfo>)),
            this, SLOT(showSearchResult(QList<ItemInfo>)));

    connect(signalManager, SIGNAL(mouseReleased()), this, SLOT(handleMouseReleased()));
    connect(signalManager, SIGNAL(Hide()), this, SLOT(Hide()));
    connect(signalManager, SIGNAL(appOpened(QString)), this, SLOT(handleAppOpened(QString)));
    connect(signalManager, SIGNAL(itemDeleted(QString)), this, SLOT(uninstallUpdateTable(QString)));
    connect(signalManager, SIGNAL(rightClickedChanged(bool)), this, SLOT(setRightclicked(bool)));
    connect(signalManager, SIGNAL(screenGeometryChanged()), this , SLOT(handleScreenGeometryChanged()));
    connect(signalManager, SIGNAL(activeWindowChanged(uint)), this, SLOT(handleActiveWindowChanged(uint)));
    connect(signalManager, SIGNAL(appItemDragStateChanged(bool)), this, SLOT(setDragging(bool)));
    connect(qApp, SIGNAL(aboutToQuit()), this, SIGNAL(Closed()));
    connect(m_categoryFrame, SIGNAL(showed()), this, SLOT(showGradients()));
    connect(m_categoryFrame, SIGNAL(contentScrolled(int)), this, SLOT(handleContentScrolled(int)));
    connect(m_appTableWidget, SIGNAL(showed()), this, SLOT(showGradients()));
    connect(m_appTableWidget, SIGNAL(contentScrolled(int)), this, SLOT(handleContentScrolled(int)));
    connect(m_backgroundLabel, SIGNAL(changed(QPixmap)), this, SLOT(updateGradients(QPixmap)));
}

int LauncherFrame::currentMode(){
    return m_layout->currentIndex();
}

void LauncherFrame::toggleDisableNavgationBar(bool flag){
    m_categoryFrame->getNavigationBar()->setDisabled(flag);
}

void LauncherFrame::showSortedMode(int mode){
//    qDebug() << mode;
    if (mode == 1){
        showNavigationBarByMode();
        showCategoryTable();
    }else{
        showAppTableWidgetByMode(mode);
    }
}

void LauncherFrame::showCategoryMode(int mode){
//   qDebug() << mode;
   m_categoryFrame->getNavigationBar()->setCurrentIndex(mode);
   showCategoryTable();
}

void LauncherFrame::showAppTableWidget(){
    m_currentIndex = currentMode();
    m_layout->setCurrentIndex(1);
    m_displayModeFrame->hide();
}

void LauncherFrame::showCategoryTable()
{
    m_categoryFrame->getCategoryTabelWidget()->setCategoryInfoList(appItemManager->getSortedCategoryInfoList());
}

void LauncherFrame::showSearchResult(const QList<ItemInfo> &infoList){
    showAppTableWidget();
    m_appTableWidget->showSearchResult(infoList);
}

void LauncherFrame::showAppTableWidgetByMode(int mode){
    m_layout->setCurrentIndex(1);
    m_displayModeFrame->show();
    m_displayModeFrame->raise();
    m_appTableWidget->showBySortedMode(mode);
}

void LauncherFrame::showNavigationBarByMode(){
    m_layout->setCurrentIndex(0);
    m_displayModeFrame->show();
    m_displayModeFrame->raise();
}

void LauncherFrame::mouseReleaseEvent(QMouseEvent *event){
    qDebug() << event;
    if (event->button() == Qt::LeftButton){
        bool flag = LauncherApp::UnistallAppNames.count() == 0;
        if (!m_displayModeFrame->getButtonFrame()->isVisible() && flag){
            emit signalManager->mouseReleased();
            Hide();
        }
    }
    QFrame::mouseReleaseEvent(event);
}

void LauncherFrame::keyPressEvent(QKeyEvent *event){
    QFrame::keyPressEvent(event);
}

void LauncherFrame::closeEvent(QCloseEvent *event){
    qDebug() << event;
    QDBusConnection conn = QDBusConnection::sessionBus();
    conn.unregisterObject("/com/deepin/dde/Launcher");
    conn.unregisterService("com.deepin.dde.Launcher");
    QFrame::closeEvent(event);
}


void LauncherFrame::setRightclicked(bool flag){
    m_rightclicked = flag;
}

void LauncherFrame::handleScreenGeometryChanged(){
//    QFileInfo fileInfo(m_backgroundLabel->getCacheUrl());
//    if (fileInfo.exists()){
//        bool flag = QFile::remove(m_backgroundLabel->getCacheUrl());
//        qDebug() << "remove" << m_backgroundLabel->getCacheUrl() << flag;
//    }
    qApp->quit();
}

void LauncherFrame::handleContentScrolled(int value)
{
    if (value == 0) {
        m_topGradient->setVisible(false);
    } else {
        m_topGradient->setVisible(true);
    }
}

void LauncherFrame::Exit(){
    qDebug() << "LauncherFrame::Exit()";
    close();
    qApp->quit();
}

void LauncherFrame::Hide(){
    hide();
    emit Closed();
    m_searchLineEdit->setText("");
    emit signalManager->refreshInstallTimeFrequency();
    emit signalManager->firstButtonChecked();
}

void LauncherFrame::Show(){
    qDebug() << "launcher frame show or not:" << this->isHidden();

    DBusControlCenter* controlCenterInterface = new DBusControlCenter(this);
    if (controlCenterInterface->isValid()) {
        controlCenterInterface->Hide();
        qDebug() << "dcc Inter:" << controlCenterInterface->isValid();
    } else {
        qDebug() << "dcc Inter is inValid!";
    }


    move(m_primaryPos);
    m_rightclicked = false;
    emit signalManager->hightlightButtonByIndex(0);
    show();
    m_searchLineEdit->setSearchFocus();
    raise();
    activateWindow();
    emit Shown();
    qDebug() << geometry();
    controlCenterInterface->deleteLater();

}

void LauncherFrame::ShowByMode(qlonglong mode)
{
    emit signalManager->viewModeChanged(mode);
    Show();
}

void LauncherFrame::Toggle(){
    handleToggle();
}

void LauncherFrame::handleToggle(){
    if (isVisible()){
        Hide();
    }else{
        Show();
    }
}

void LauncherFrame::handleMouseReleased(){
    emit signalManager->highlightChanged(false);
}

void LauncherFrame::handleSearch(const QString &text){
    if (text.length() == 0){
        m_searchTimer->stop();
        clearSearchEdit();
    }else{
        m_searchTimer->start();
    }
}

void LauncherFrame::search(){
    emit signalManager->search(m_searchLineEdit->getText());
}

void LauncherFrame::clearSearchEdit(){
    m_searchLineEdit->setText("");
    backNormalView();
}

void LauncherFrame::backNormalView(){
    int mode = dbusController->getSortMethod();
    showSortedMode(mode);
}

void LauncherFrame::handleAppOpened(const QString &appUrl){
    Q_UNUSED(appUrl)
    Hide();
}

void LauncherFrame::uninstallUpdateTable(QString appKey){
    qDebug() << appKey;
}

LauncherFrame::~LauncherFrame()
{
    qDebug() << "~LauncherFrame";
}

void LauncherFrame::showGradients() const
{
    if (m_categoryFrame->isVisible()) {
        QRect top = m_categoryFrame->topGradientRect();
        m_topGradient->setPixmap(m_backgroundLabel->pixmap()->copy(top));
        m_topGradient->resize(top.size());
        m_topGradient->move(top.topLeft());
        m_topGradient->raise();

        QRect bottom = m_categoryFrame->bottomGradientRect();
        m_bottomGradient->setPixmap(m_backgroundLabel->pixmap()->copy(bottom));
        m_bottomGradient->resize(bottom.size());
        m_bottomGradient->move(bottom.topLeft());
        m_bottomGradient->raise();
    } else if (m_appTableWidget->isVisible()) {
        QSize size(m_appTableWidget->width(), TopBottomGradientHeight);
        QPoint topLeft = m_appTableWidget->mapTo(this, QPoint(0, 0));
        QRect top(topLeft, size);

        m_topGradient->setPixmap(m_backgroundLabel->pixmap()->copy(top));
        m_topGradient->resize(top.size());
        m_topGradient->move(top.topLeft());
        m_topGradient->raise();

        // in case that top gradient is invisible.
        m_topGradient->show();

        QPoint bottomLeft = m_appTableWidget->mapTo(this, m_appTableWidget->rect().bottomLeft());
        // FIXME: workaround here, to fix the bug that the bottom gradient
        // if one pixel above the bottom of m_appTableWidget.
        QRect bottom(bottomLeft.x(), bottomLeft.y() + 1 - size.height(), size.width(), size.height());

        m_bottomGradient->setPixmap(m_backgroundLabel->pixmap()->copy(bottom));
        m_bottomGradient->resize(bottom.size());
        m_bottomGradient->move(bottom.topLeft());
        m_bottomGradient->raise();
    }
}


void LauncherFrame::updateGradients(QPixmap) const
{
    showGradients();
}


void LauncherFrame::inputMethodEvent(QInputMethodEvent *event){
    qDebug() << event;
    QFrame::inputMethodEvent(event);
}

void LauncherFrame::wheelEvent(QWheelEvent *event){
    if (m_layout->currentIndex() == 0){
        m_categoryFrame->getCategoryTabelWidget()->handleWheelEvent(event);
    }else{
        m_appTableWidget->handleWheelEvent(event);
    }
    QFrame::wheelEvent(event);
}

bool LauncherFrame::eventFilter(QObject *obj, QEvent *event){
    qDebug() << event;
    if (event->type() == QEvent::WindowDeactivate){
        qDebug() << "============" << obj << event;
    }
    return QFrame::eventFilter(obj, event);
}

void LauncherFrame::handleActiveWindowChanged(uint windowId){
    if (!isVisible() || windowId == window()->winId()){
        return;
    }
    qDebug() << windowId << window()->winId() << m_isDraging;
    QDBusPendingReply<qulonglong> reply = dbusController->getDockInterface()->Xid();
    reply.waitForFinished();
    if (!reply.isError()){
        WId dockwinId = reply.argumentAt(0).toLongLong();
        if (windowId != window()->winId() && windowId!= dockwinId && !m_isDraging){
                Hide();
        }else{
            emit signalManager->highlightChanged(false);
        }
    }else{
        qCritical() << reply.error().name() << reply.error().message();
    }
}

void LauncherFrame::setDragging(bool flag){
    m_isDraging = flag;
}
