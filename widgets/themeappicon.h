#ifndef THEMEAPPICON_H
#define THEMEAPPICON_H

#include <QObject>

class ThemeAppIcon : public QObject
{
    Q_OBJECT
public:
    explicit ThemeAppIcon(QObject *parent = 0);
    ~ThemeAppIcon();

    void gtkInit();

    static QPixmap getIconPixmap(QString iconPath, int width=64, int height=64);
    static QString getThemeIconPath(QString iconName);

signals:

public slots:
};

#endif // THEMEAPPICON_H
