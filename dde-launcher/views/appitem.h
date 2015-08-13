#ifndef APPITEM_H
#define APPITEM_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>


class ElidedLabel;

class AppItem : public QFrame
{
    Q_OBJECT
public:

    explicit AppItem(QWidget *parent = 0);
    explicit AppItem(QString icon, QString name, QWidget *parent = 0);
    explicit AppItem(QPixmap icon, QString name, QWidget *parent = 0);
    explicit AppItem(QString url, QString icon, QString name, QWidget *parent = 0);
    ~AppItem();

    void initUI();
    void initConnect();

    QPixmap getAppIcon();
    QString getAppName();
    QString getUrl();

signals:

public slots:
    void setAppIcon(QString icon);
    void setAppIcon(QPixmap& icon);
    void setAppName(QString name);
    void setUrl(QString url);

protected:
    void mousePressEvent(QMouseEvent* event);
    void resizeEvent(QResizeEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);

private:
    QLabel* m_iconLabel;
    QLabel* m_borderLabel;
    ElidedLabel* m_nameLabel;
    QPixmap m_appIcon;
    QString m_appName = "";
    QString m_url;
};

#endif // APPITEM_H
