#include <QMessageBox>

#include "../common/countries/CountryManager.h"

#include "model/orderimporters/Shipment.h"
#include "model/bookkeeping/ManagerSaleTypes.h"

#include "DialogAddVatSaleAccount.h"
#include "ui_DialogAddVatSaleAccount.h"

//----------------------------------------------------------
DialogAddVatSaleAccount::DialogAddVatSaleAccount(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddVatSaleAccount)
{
    ui->setupUi(this);
    ui->comboRegime->addItems(*Shipment::allRegimes());
    ui->comboCountry->addItem(CountryManager::EU);
    ui->comboCountry->addItems(*CountryManager::countriesNamesUEfrom2020());
    ui->comboTypeSale->addItems(
                {ManagerSaleTypes::SALE_PRODUCTS,
                 ManagerSaleTypes::SALE_SERVICES,
                 ManagerSaleTypes::SALE_PAYMENT_FASCILITOR});
    m_wasAccepted = false;
}
//----------------------------------------------------------
DialogAddVatSaleAccount::~DialogAddVatSaleAccount()
{
    delete ui;
}
//----------------------------------------------------------
QString DialogAddVatSaleAccount::getRegime() const
{
    return ui->comboRegime->currentText();
}
//----------------------------------------------------------
QString DialogAddVatSaleAccount::getCountryName() const
{
    return ui->comboCountry->currentText();
}
//----------------------------------------------------------
QString DialogAddVatSaleAccount::getSaleType() const
{
    return ui->comboTypeSale->currentText();
}
//----------------------------------------------------------
double DialogAddVatSaleAccount::getVatRate() const
{
    return ui->spinBoxVatRate->value();
}
//----------------------------------------------------------
QString DialogAddVatSaleAccount::getVatRateString() const
{
    return SettingManager::formatVatRate(ui->spinBoxVatRate->value());
}
//----------------------------------------------------------
QString DialogAddVatSaleAccount::getAccountSale() const
{
    return ui->lineEditAccountSale->text();
}
//----------------------------------------------------------
QString DialogAddVatSaleAccount::getAccountVat() const
{
    return ui->lineEditAccountVat->text();
}
//----------------------------------------------------------
void DialogAddVatSaleAccount::accept()
{
    if (ui->lineEditAccountSale->text().isEmpty()) {
        QMessageBox::warning(
                    this,
                    tr("Compte de vente manquant"),
                    tr("Vous devez saisir un compte de vente"));
    } else if (ui->lineEditAccountSale->text().isEmpty()) {
        QMessageBox::warning(
                    this,
                    tr("Compte de TVA manquant"),
                    tr("Vous devez saisir un compte de TVA"));
    } else {
        m_wasAccepted = true;
        QDialog::accept();
    }
}
//----------------------------------------------------------
void DialogAddVatSaleAccount::reject()
{
    m_wasAccepted = false;
    QDialog::reject();
}
//----------------------------------------------------------
bool DialogAddVatSaleAccount::wasAccepted() const
{
    return m_wasAccepted;
}
//----------------------------------------------------------
