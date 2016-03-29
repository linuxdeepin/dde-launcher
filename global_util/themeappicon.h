#ifndef THEMEAPPICON_H
#define THEMEAPPICON_H

#include <QObject>

#undef signals
extern "C" {
    #include <string.h>
    #include <gtk/gtk.h>
    #include <gio/gdesktopappinfo.h>
}
#define signals public

class ThemeAppIcon : public QObject
{
    Q_OBJECT
public:
    explicit ThemeAppIcon(QObject *parent = 0);
    ~ThemeAppIcon();

    void gtkInit();

    static QPixmap getIconPixmap(QString iconPath, int width=64, int height=64);
    static QString getThemeIconPath(QString iconName, int size=64);

signals:

public slots:
};

#endif // THEMEAPPICON_H
