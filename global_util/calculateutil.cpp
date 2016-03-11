#include "calcuateutil.h"
#include <QtCore>

int getRowCount(int appListCounts, int columnCounts) {
    qDebug() << "appListCounts" << appListCounts << columnCounts;
    if (columnCounts <= 0 || appListCounts <=0) {
        return 0;
    } else {
        int rowCount = appListCounts/columnCounts;
        qDebug() << "rowCount:" << rowCount;
        if (appListCounts%columnCounts==0) {
            return rowCount;
        } else {
            return (rowCount+1);
        }
        return 0;

        return 0;
    }
    return 0;

}
