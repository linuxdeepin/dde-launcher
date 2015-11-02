#ifndef SYSTEMBACKGROUND_H
#define SYSTEMBACKGROUND_H

#include <QObject>
#include <QPixmap>
#include <QGSettings>

class SystemBackground : public QObject
{
    Q_OBJECT
public:
    explicit SystemBackground(QSize size, bool isBlur=false, QObject *parent = 0);
    static const QString WallpaperKey;
    void initConnect();
    bool isBlur() const ;
    QSize getBackgroundSize() const;
    const QPixmap& getBackground();

    QString joinPath(const QString& path, const QString& fileName);
    QString getBackgroundsPath();
    QString getCacheUrl();
    QRect getPreferScaleClipRect(int refWidth, int refHeight, int width, int height);
signals:
    void backgroundChanged(const QPixmap& backgroundPixmap);

public slots:
    void updateBackgroud();
    void handleBackgroundChanged(const QString &key);
    void setBlur(bool isBlur);
    void setBackgroundSize(QSize size);

private:
    QSize m_backgroundSize;
    bool m_isBlur;
    QGSettings* m_gsettings;
    QString m_backgroundUrl;
    QString m_cacheUrl;
    QPixmap m_backgroundPixmap;
};

#endif // SYSTEMBACKGROUND_H
