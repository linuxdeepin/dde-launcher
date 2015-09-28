#ifndef DBUSWORKER_H
#define DBUSWORKER_H

#include <QObject>

class DBusWorker : public QObject
{
    Q_OBJECT
public:
    explicit DBusWorker(QObject *parent = 0);
    void initConnect();
signals:

public slots:
    void requsetData();

};

#endif // DBUSWORKER_H
