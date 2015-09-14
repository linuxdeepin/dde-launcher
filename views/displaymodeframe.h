#ifndef DISPLAYMODEFRAME_H
#define DISPLAYMODEFRAME_H

#include <QFrame>


class BaseCheckedButton;
class QButtonGroup;
class BaseFrame;
class ViewModeButton;
class QTimer;
class QPropertyAnimation;

class DisplayModeFrame : public QFrame
{
    Q_OBJECT
public:
    explicit DisplayModeFrame(QWidget *parent = 0);
    ~DisplayModeFrame();

    void initUI();
    void initConnect();
    int getCategoryDisplayMode();
    int getSortMode();


signals:
    void categoryModeChanged(int mode);
    void sortModeChanged(int mode);
    void visibleChanged(bool isVisible);

public slots:
    void setCategoryDisplayMode(int mode);
    void setSortMode(int mode);
    void checkButton(int mode);
    void setViewMode(int mode);

    void toggleButtonFrameByViewButton(bool flag);
    void toggleButtonFrameBySelf(bool flag);
    void showButtonFrame();
    void hideButtonFrame();


private:
    int m_categoryDisplayMode;
    int m_sortMode;
    int m_viewMode = 1;

    ViewModeButton* m_viewModeButton;
    BaseFrame* m_buttonFrame;
    BaseCheckedButton* m_nameButton;
    BaseCheckedButton* m_categoryIconButton;
    BaseCheckedButton* m_categoryNameButton;
    BaseCheckedButton* m_installTimeButton;
    BaseCheckedButton* m_useFrequencyButton;
    QButtonGroup* m_buttonGroup;
    QTimer* m_delayHideTimer;
};

#endif // DISPLAYMODEFRAME_H
