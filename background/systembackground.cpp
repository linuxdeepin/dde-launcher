#include "systembackground.h"
#include <QPainter>
#include <QImage>
#include <QVariant>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
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

void SystemBackground::updateBackgroud(){
    QString lastModifiedtime = QString::number(QFileInfo(m_backgroundUrl).lastModified().toMSecsSinceEpoch());
    QString cacheUrl = joinPath(getBackgroundsPath(),
                                QString("%1_%2.png").arg(
                                    QFileInfo(m_backgroundUrl).baseName(), lastModifiedtime));
    if (QFileInfo(cacheUrl).exists()){
        qDebug() << cacheUrl;
        m_backgroundPixmap = QPixmap(cacheUrl);
    }else{
        m_backgroundPixmap = QPixmap(m_backgroundUrl);
        QPixmap tempPixmap = m_backgroundPixmap.scaled(m_backgroundSize);
        if (m_isBlur){
            QPainter painter;
            painter.begin(&tempPixmap);
            QImage backgroundImage = tempPixmap.toImage();
            qt_blurImage(&painter, backgroundImage, 100, false, false);
            painter.end();
        }
        m_backgroundPixmap = tempPixmap;
        m_backgroundPixmap.save(cacheUrl);
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
