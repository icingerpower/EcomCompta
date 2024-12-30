#include <qmessagebox.h>

#include "DialogNewCustomer.h"
#include "ui_DialogNewCustomer.h"

#include "model/CustomerManager.h"
#include "model/SettingManager.h"

//----------------------------------------------------------
DialogNewCustomer::DialogNewCustomer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNewCustomer)
{
    ui->setupUi(this);
    ui->comboCurrency->addItems(SettingManager::instance()->currencies());
    m_blockCancel = false;
}
//----------------------------------------------------------
DialogNewCustomer::~DialogNewCustomer()
{
    delete ui;
}
//----------------------------------------------------------
Customer DialogNewCustomer::getCustomer() const
{
    Customer customer(
                ui->lineEditName->text(),
                ui->plainTextEditNotes->toPlainText(),
                ui->lineEditSiret->text(),
                ui->lineEditPhonNumber->text(),
                ui->lineEditEmail->text(),
                ui->comboCurrency->currentText());
    return customer;
}
//----------------------------------------------------------
Address DialogNewCustomer::getAddress() const
{
    Address address = ui->widgetAddress->getAddress();
    return address;
}
//----------------------------------------------------------
void DialogNewCustomer::clear()
{
    ui->lineEditName->clear();
    ui->plainTextEditNotes->clear();
    ui->lineEditSiret->clear();
    ui->lineEditPhonNumber->clear();
    ui->lineEditEmail->clear();
}
//----------------------------------------------------------
void DialogNewCustomer::blockCancel()
{
    ui->labelInstructions->setText(
                tr("Avant d'utiliser l'application, vous devez ajouter le premier client."));
    m_blockCancel = true;
}
//----------------------------------------------------------
void DialogNewCustomer::releaseCancel()
{
    ui->labelInstructions->setText("");
    m_blockCancel = false;
}
//----------------------------------------------------------
void DialogNewCustomer::accept()
{
    QString customerName = ui->lineEditName->text().trimmed();
    bool isAddressComplete = ui->widgetAddress->isAddressComplete();
    if (customerName.isEmpty()) {
        QMessageBox::warning(
                    this,
                    tr("Erreur"),
                    tr("Le nom du client doit être saisi."));
    } else if (!CustomerManager::instance()->isCustomerAvailable(customerName)) {
        QMessageBox::warning(
                    this,
                    tr("Erreur"),
                    tr("Le nom du client est déjà utilisé."));
    } else if (!isAddressComplete) {
        QMessageBox::warning(
                    this,
                    tr("Erreur"),
                    tr("L'adresse doit au moins renseigner le pays et avoir un label (pour son identification)."));
    } else {
        QDialog::accept();
    }
}
//----------------------------------------------------------
void DialogNewCustomer::reject()
{
    if (m_blockCancel) {
        QMessageBox::warning(
                    this,
                    tr("Erreur"),
                    tr("Pour utiliser l'application, vous devez au moins créer un client avec une adresse."));
        QWidget *parent = this;
        while (parent->parentWidget() != nullptr) {
            parent = parent->parentWidget();
        }
        parent->close();
    } else {
        QDialog::reject();
    }
}
//----------------------------------------------------------

