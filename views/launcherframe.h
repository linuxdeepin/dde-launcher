#ifndef LAUNCHERFRAME_H
#define LAUNCHERFRAME_H

#include <QFrame>
#include <QMouseEvent>

class CategoryFrame;
class AppTableWidget;
class DisplayModeFrame;
class QStackedLayout;
class QButtonGroup;
class QPushButton;
class QCloseEvent;
class SearchLineEdit;

class LauncherFrame : public QFrame
{
    Q_OBJECT
public:
    explicit LauncherFrame(QWidget *parent = 0);
    ~LauncherFrame();

    static QButtonGroup buttonGroup;

    void initUI();
    void initConnect();
    void computerGrid(int minimumLeftMargin, int minimumTopMargin, int miniSpacing ,int itemWidth);

signals:
    void Closed();
    void Shown();

public slots:
    void showSortedMode(int mode);
    void showCategoryMode(int mode);
    void toggleDisableNavgationBar(bool flag);
    void showAppTableWidgetByMode(int mode);
    void showNavigationBarByMode();
    void handleMouseReleased();
    void showAppTableWidget();
    void Exit();
    void Hide();
    void Show();
    void Toggle();
    void handleSearch(const QString& text);
    void hideSearchEdit();

    void handleAppOpened(const QString& appKey);

protected:
    void mouseReleaseEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void closeEvent(QCloseEvent* event);

private:
    int m_leftMargin;
    int m_rightMargin;
    int m_topMargin;
    int m_bottomMargin;
    int m_spacing;
    int m_column;
    int m_row;
    int m_itemWidth;
    int m_gridwidth;
    QStackedLayout* m_layout;
    DisplayModeFrame* m_displayModeFrame;
    CategoryFrame* m_categoryFrame;
    AppTableWidget* m_appTableWidget;

    QPushButton* m_clearCheckedButton;

    SearchLineEdit* m_searchLineEdit;
};

#endif // LAUNCHERFRAME_H
