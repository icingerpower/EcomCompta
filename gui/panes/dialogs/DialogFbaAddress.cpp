#include <qmessagebox.h>
#include "DialogFbaAddress.h"
#include "ui_DialogFbaAddress.h"

#include "model/vat/VatRatesModel.h"
#include "model/SettingManager.h"
#include "model/vat/AmazonFulfillmentAddressModel.h"

//----------------------------------------------------------
DialogFbaAddress::DialogFbaAddress(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogFbaAddress)
{
    ui->setupUi(this);
    ui->comboBoxCountry->addItem("-");
    ui->comboBoxCountry->addItems(
                *SettingManager::countriesUEfrom2020());
}
//----------------------------------------------------------
DialogFbaAddress::~DialogFbaAddress()
{
    delete ui;
}
//----------------------------------------------------------
Address DialogFbaAddress::getAddress() const
{
    Address address;
    address.setFullName(ui->lineEditName->text().trimmed());
    address.setPostalCode(ui->lineEditPostalCode->text().trimmed());
    address.setCity(ui->lineEditCity->text().trimmed());
    address.setState(ui->lineEditState->text().trimmed());
    address.setCountryCode(ui->comboBoxCountry->currentText());
    return address;
}
//----------------------------------------------------------
void DialogFbaAddress::clear()
{
    ui->lineEditCity->clear();
    ui->lineEditPostalCode->clear();
    ui->lineEditCity->clear();
    ui->lineEditState->clear();
    ui->comboBoxCountry->setCurrentIndex(0);
}
//----------------------------------------------------------
void DialogFbaAddress::accept()
{
    QString fbaName = ui->lineEditName->text().trimmed();
    QString country = ui->comboBoxCountry->currentText();
    if (fbaName.size() < 3) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir le nom du centre FBA"));
    } else if (country.contains("-")) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut sélectionner un pays."));
    } else if (AmazonFulfillmentAddressModel::instance()
               ->contains(fbaName)) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Le centre FBA existe déjà."));
    } else {
        QDialog::accept();
    }
}
//----------------------------------------------------------
