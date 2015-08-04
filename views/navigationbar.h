#ifndef NAVIGATIONBAR_H
#define NAVIGATIONBAR_H

#include <QFrame>
class NavigationButtonFrame;
class QStackedLayout;

class NavigationBar : public QFrame
{
    Q_OBJECT
public:
    explicit NavigationBar(QWidget *parent = 0);
    ~NavigationBar();

    void initUI(int width);
    void initConnect();
    NavigationButtonFrame* getIconFrame();
    NavigationButtonFrame* getTextFrame();

signals:

public slots:
    void setCurrentIndex(int index);

private:
    QStackedLayout* m_layout;
    NavigationButtonFrame* m_iconFrame;
    NavigationButtonFrame* m_textFrame;
};

#endif // NAVIGATIONBAR_H
