#ifndef LAUNCHERFRAME_H
#define LAUNCHERFRAME_H

#include <QFrame>
#include <QMouseEvent>

class CategoryFrame;
class AppTableWidget;
class DisplayModeFrame;
class QStackedLayout;

class LauncherFrame : public QFrame
{
    Q_OBJECT
public:
    explicit LauncherFrame(QWidget *parent = 0);
    ~LauncherFrame();

    void initUI();
    void initConnect();
    void computerGrid(int minimumLeftMargin, int minimumTopMargin, int miniSpacing ,int itemWidth);

signals:

public slots:
       void toggleDisableNavgationBar(bool flag);
       void showAppTableWidgetByMode(int mode);
       void showNavigationBarByMode(int mode);

protected:
    void mouseReleaseEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);

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
};

#endif // LAUNCHERFRAME_H
