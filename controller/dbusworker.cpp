#include "dbusworker.h"
#include "app/global.h"

DBusWorker::DBusWorker(QObject *parent) :
    QObject(parent)
{
    initConnect();
}

void DBusWorker::initConnect(){
    connect(signalManager, SIGNAL(requestData()),
            this, SLOT(requsetData()));
}

void DBusWorker::requsetData(){
    dbusController->init();
}
