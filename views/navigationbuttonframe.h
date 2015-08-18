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

public slots:
    void hideButtons(const QStringList& keys);
    void handleButtonClicked(int id);
    void checkButtonByKey(QString key);

private:
    QStringList m_categroyKeys;
    QButtonGroup* m_buttonGroup;
};

#endif // NAVIGATIONBUTTONFRAME_H
