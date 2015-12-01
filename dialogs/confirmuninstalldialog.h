#ifndef CONFIRMUNINSTALLDIALOG_H
#define CONFIRMUNINSTALLDIALOG_H

#include "dbasedialog.h"

class ConfirmUninstallDialog : public DBaseDialog
{
    Q_OBJECT

public:
    explicit ConfirmUninstallDialog(QWidget *parent = 0);
    ~ConfirmUninstallDialog();

    QString getAppKey();

signals:

public slots:
    void handleKeyEnter();
    void setAppKey(QString appKey);

private:
    QString m_appKey;
};

#endif // CONFIRMUNINSTALLDIALOG_H
