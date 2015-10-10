#ifndef NAVIGATIONBUTTONFRAME_H
#define NAVIGATIONBUTTONFRAME_H

#include <QStringList>
#include <QFrame>

class BaseCheckedButton;
class QButtonGroup;

class NavigationButtonFrame : public QFrame
{
    Q_OBJECT
public:
    explicit NavigationButtonFrame(int mode, QWidget *parent = 0);
    ~NavigationButtonFrame();

    void initConnect();
    void initByMode(int mode);

signals:
    void currentIndexChanged(int index);
public slots:
    void hideButtons(const QStringList& keys);
    void handleButtonClicked(int id);
    void checkButtonByKey(QString key);
    void setCurrentIndex(int currentIndex);
    void addTextShadow();

    void checkFirstButton();

private:
    QButtonGroup* m_buttonGroup;
    int m_currentIndex = 0;
};

#endif // NAVIGATIONBUTTONFRAME_H
