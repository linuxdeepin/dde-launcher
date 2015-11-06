#include "systembackground.h"
#include <QPainter>
#include <QImage>
#include <QVariant>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QDesktopWidget>
#include <QApplication>
#include <QDebug>

QT_BEGIN_NAMESPACE
extern Q_GUI_EXPORT void qt_blurImage( QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0 );
QT_END_NAMESPACE

const QString SystemBackground::WallpaperKey = "pictureUri";

SystemBackground::SystemBackground(QSize size, bool isBlur, QObject *parent):
    QObject(parent),
    m_backgroundSize(size),
    m_isBlur(isBlur),
    m_gsettings(new QGSettings("com.deepin.wrap.gnome.desktop.background",
                               "/com/deepin/wrap/gnome/desktop/background/"))
{
    m_backgroundUrl = QUrl(m_gsettings->get(WallpaperKey).toString()).toLocalFile();
    updateBackgroud();
    initConnect();
}

void SystemBackground::initConnect(){
    connect(m_gsettings, SIGNAL(changed(QString)),
            this, SLOT(handleBackgroundChanged(QString)));
}

bool SystemBackground::isBlur() const{
    return m_isBlur;
}

void SystemBackground::setBlur(bool isBlur){
    m_isBlur = isBlur;
    updateBackgroud();
}

QSize SystemBackground::getBackgroundSize() const {
    return m_backgroundSize;
}

void SystemBackground::setBackgroundSize(QSize size){
    m_backgroundSize = size;
    updateBackgroud();
}

const QPixmap& SystemBackground::getBackground(){
    return m_backgroundPixmap;
}

QString SystemBackground::getCacheUrl(){
    return m_cacheUrl;
}

void SystemBackground::updateBackgroud(){
    QString lastModifiedtime = QString::number(QFileInfo(m_backgroundUrl).lastModified().toMSecsSinceEpoch());
    m_cacheUrl = joinPath(getBackgroundsPath(),
                          QString("%1_%2.png").arg(
                              QFileInfo(m_backgroundUrl).baseName(), lastModifiedtime));

    if (QFileInfo(m_cacheUrl).exists()){
        qDebug() << m_cacheUrl;
        m_backgroundPixmap = QPixmap(m_cacheUrl);
    }else{
        m_backgroundPixmap = QPixmap(m_backgroundUrl);

        int sideEffectInnerGlowRadius = 100;
        int blurRadius = 100;
        int refWidth = m_backgroundSize.width();
        int refHeight = m_backgroundSize.height();
        int imgWidth = m_backgroundPixmap.size().width();
        int imgHeight = m_backgroundPixmap.size().height();

        QRect r = getPreferScaleClipRect(refWidth, refHeight, imgWidth, imgHeight);

        QPixmap blurPixmap(QSize(r.width(), r.height()));
        QPixmap tempPixmap(QSize(r.width() + sideEffectInnerGlowRadius,
                                 r.height() + sideEffectInnerGlowRadius));
        blurPixmap.fill(Qt::transparent);
        tempPixmap.fill(Qt::transparent);

        if (m_isBlur){
            QPainter painter1;
            painter1.begin(&tempPixmap);
            // qt_blurImage has problems processing the border part of images.
            // We can first fill the background with the scaled one, providing
            // some clues to qt_blurImage
            painter1.drawPixmap(0, 0, m_backgroundPixmap.scaled(tempPixmap.size()));
            painter1.drawPixmap(sideEffectInnerGlowRadius / 2,
                                sideEffectInnerGlowRadius / 2,
                                m_backgroundPixmap,
                                r.x(), r.y(), r.width(), r.height());
            painter1.end();

            QPainter painter2;
            painter2.begin(&tempPixmap);
            QImage backgroundImage = tempPixmap.toImage();
            qt_blurImage(&painter2, backgroundImage, blurRadius, false, false);
            painter2.end();

            QPainter painter3;
            painter3.begin(&blurPixmap);
            painter3.drawPixmap(0, 0, tempPixmap,
                                sideEffectInnerGlowRadius / 2,
                                sideEffectInnerGlowRadius / 2,
                                r.width(), r.height());
            painter3.end();
        }
        m_backgroundPixmap = blurPixmap.scaled(m_backgroundSize);
        m_backgroundPixmap.save(m_cacheUrl);
    }
    emit backgroundChanged(m_backgroundPixmap);
}

void SystemBackground::handleBackgroundChanged(const QString &key){
    if (key == WallpaperKey){
        m_backgroundUrl = QUrl(m_gsettings->get(WallpaperKey).toString()).toLocalFile();
        qDebug() << "background changed: " << m_backgroundUrl;
        updateBackgroud();
    }
}

QRect SystemBackground::getPreferScaleClipRect(int refWidth, int refHeight, int imgWidth, int imgHeight){
    if (refWidth*refHeight == 0 || imgWidth * imgHeight == 0){
        return QRect(0, 0, 0, 0);
    }
    float scale = float(refWidth) / float(refHeight);
    int w = imgWidth;
    int h = int(float(w) / scale);
    int x = 0, y = 0;
    if (h < imgHeight){
        y = (imgHeight - h) / 2;
    }else{
        h = imgHeight;
        w = int(float(h) * scale);
        x = (imgWidth - w) / 2;
    }
    return QRect(x, y, w, h);
}

QString SystemBackground::joinPath(const QString& path, const QString& fileName){
    QString separator(QDir::separator());
    return QString("%1%2%3").arg(path, separator, fileName);
}

QString SystemBackground::getBackgroundsPath(){
    QString cachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).at(0);
    QString backgroundPath = joinPath(cachePath, "background");
    if (!QDir(backgroundPath).exists()){
        QDir(backgroundPath).mkpath(backgroundPath);
    }
    return backgroundPath;
}
