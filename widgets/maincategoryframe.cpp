#include "maincategoryframe.h"
#include "global_util/global.h"


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

    updateUI();
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

MainCategoryFrame::~MainCategoryFrame(){

}
