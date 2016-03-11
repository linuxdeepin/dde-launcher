#include "maincategoryframe.h"


MainCategoryFrame::MainCategoryFrame(QFrame *parent)
    : QFrame(parent) {
    AppItemDelegate* itemDelegate = new AppItemDelegate(this);
    for(int i = 0; i < 11; i++) {
        CategoryTitleWidget* tmpCategoryTitleWidget = new CategoryTitleWidget(CategoryKeys.at(i),this);
        m_cateTitleWidgetList.append(tmpCategoryTitleWidget);
        AppListView* tmpListView = new AppListView(this);
        m_listViewList.append(tmpListView);
        AppsListModel* tmpListModel = new AppsListModel(CategoryID(i), this);
        tmpListModel->setListModelData(CategoryID(i));
        m_listModelList.append(tmpListModel);

        tmpListView->setModel(tmpListModel);
        tmpListView->setItemDelegate(itemDelegate);
        qDebug() << "CategoryKeys.at(i)" << i << CategoryKeys.at(i);
    }

    m_layout = new QVBoxLayout;
    m_layout->addSpacing(0);
    m_layout->addStretch();
    for (int i = 0; i < 11; i++) {
        m_layout->addWidget(m_cateTitleWidgetList[i]);
        m_layout->addWidget(m_listViewList[i]);
    }
    m_layout->addStretch();

    setLayout(m_layout);
    setStyleSheet("background-color: rgba(255, 222, 173,255);");

    m_bottonFrame = new QFrame(this);
    m_bottonFrame->setStyleSheet("background-color:transparent;");
    updateUI();
    initConnect();
    addAnEmptyFrameInBottom();

}

void MainCategoryFrame::scrollToCategory(int index) {
    QWidget *scrollDestinationWidget = nullptr;
    scrollDestinationWidget = m_cateTitleWidgetList[index];
    qDebug() << "m_cateTitleWidgetList" << index;
    // scroll to destination
    if (scrollDestinationWidget) {
        emit signalManager->scrollToValue(scrollDestinationWidget->pos().y());
    }

}

void MainCategoryFrame::initConnect() {
    connect(signalManager, &SignalManager::scrollToCategory, this, &MainCategoryFrame::scrollToCategory);

    foreach (AppListView* appListView, m_listViewList) {
        connect(appListView, &AppListView::clicked, this, &MainCategoryFrame::clickItemIndex);
    }
}

void MainCategoryFrame::updateUI() {
    QList<int> appNumList = appsManager->getCategoryAppNumsList();
    for(int i(0); i< appNumList.size();i++) {
        if (appNumList[i]==0) {
            m_cateTitleWidgetList[i]->hide();
            m_listViewList[i]->hide();

        }
    }
}

void MainCategoryFrame::addAnEmptyFrameInBottom() {
    QList<int> appNums = appsManager->getCategoryAppNumsList();
    qDebug() << "+++++++++" << appNums << "*" << appNums[appNums.length()-1] ;
    if (appNums[appNums.length()-1]) {
        int rowNum = getRowCount(appNums[appNums.length()-1], 8);

        qDebug() << "appNums others" <<  rowNum;
        //800 is the height　of the category frame's total height
        m_bottonFrame->setFixedHeight(800 - 160 - rowNum*150);
        m_layout->addWidget(m_bottonFrame);
    } else {
        int rowNum = getRowCount(appNums[appNums.length()-2], 8);

        qDebug() << "xappNums others" <<  rowNum;
        for(int i = 0; i< m_listViewList.length();i++) {
            qDebug() << "****#x" << m_listViewList[i]->height();
        }

        //800 is the height　of the category frame's total height
        m_bottonFrame->setFixedHeight(800 -160 - rowNum*150);
        m_layout->addWidget(m_bottonFrame);
    }
}

void MainCategoryFrame::clickItemIndex(QModelIndex index) {
    qDebug() << "QModelIndex:" << index.data(AppsListModel::AppIconRole).toString()
             << index.data(AppsListModel::AppNameRole).toString();
}

MainCategoryFrame::~MainCategoryFrame(){

}
