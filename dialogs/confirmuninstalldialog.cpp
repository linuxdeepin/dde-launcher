#include "confirmuninstalldialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QPushButton>

ConfirmUninstallDialog::ConfirmUninstallDialog(QWidget *parent) : DBaseDialog(parent)
{
    m_appKey = "";
    QString icon = ":/images/skin/dialogs/images/user-trash-full.png";
    QString message = "Are you sure to uninstall this application?";
    QString tipMessage = tr("All dependencies will be removed together");
    QStringList buttons, buttonTexts;
    buttons << "Cancel" << "Confirm";
    buttonTexts << tr("Cancel") << tr("Confirm");
    initUI(icon, message, tipMessage, buttons, buttons);
    moveCenter();
    getButtonsGroup()->button(1)->setFocus();
    setButtonTexts(buttonTexts);
}

void ConfirmUninstallDialog::handleKeyEnter(){
    handleButtonsClicked(1);
}

QString ConfirmUninstallDialog::getAppKey(){
    return m_appKey;
}

void ConfirmUninstallDialog::setAppKey(QString appKey){
    m_appKey = appKey;
}

ConfirmUninstallDialog::~ConfirmUninstallDialog()
{

}
