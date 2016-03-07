#ifndef CALCUATE_UTIL_H
#define CALCUATE_UTIL_H



int getRowCount(int appListCounts, int columnCounts) {
    if (columnCounts <=) {
        return 0;
    } else {
        if (appListCounts=0) {
            return 0;
        } else {
            int rowCount = appListCounts/columnCounts;
            if (appListCounts%columnCounts==0) {
                return rowCount;
            } else {
                return (rowCount+1);
            }
        }
    }
}
#endif // CALCUATE_UTIL_H
