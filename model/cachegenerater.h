#ifndef CACHEGENERATER_H
#define CACHEGENERATER_H

#include <QObject>

class CacheGenerater : public QObject
{
    Q_OBJECT
public:
    explicit CacheGenerater(QObject *parent = 0);

signals:

public slots:
};

#endif // CACHEGENERATER_H