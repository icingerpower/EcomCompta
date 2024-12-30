#include "../common/countries/CountryManager.h"

#include "WidgetAddress.h"
#include "ui_WidgetAddress.h"
#include "model/SettingManager.h"


//----------------------------------------------------------
WidgetAddress::WidgetAddress(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetAddress)
{
    ui->setupUi(this);
    QStringList countries = *CountryManager::instance()->countriesNamesUEfrom2020();
    countries.insert(0, CountryManager::instance()->countryName("CN"));
    ui->comboBoxCountryCode->addItem("");
    ui->comboBoxCountryCode->addItems(countries);
}
//----------------------------------------------------------
WidgetAddress::~WidgetAddress()
{
    delete ui;
}
//----------------------------------------------------------
QString WidgetAddress::getCountry() const
{
    QString country = ui->comboBoxCountryCode->currentText();
    return country;
}
//----------------------------------------------------------
Address WidgetAddress::getAddress() const
{
    QString countryName = ui->comboBoxCountryCode->currentText();
    QString countryCode = CountryManager::instance()->countryCode(countryName);
    Address address(
                ui->lineEditName->text().trimmed(),
                ui->lineEditStreet1->text().trimmed(),
                ui->lineEditStreet2->text().trimmed(),
                "",
                ui->lineEditCity->text().trimmed(),
                ui->lineEditPostalCode->text().trimmed(),
                countryCode,
                ui->lineEditState->text().trimmed(),
                "");
    address.setLabel(ui->lineEditLabel->text().trimmed());
    return address;
}
//----------------------------------------------------------
void WidgetAddress::setAddress(const Address &address)
{
    ui->lineEditCity->setText(address.city());
    ui->lineEditName->setText(address.fullName());
    ui->lineEditLabel->setText(address.label());
    ui->lineEditState->setText(address.state());
    ui->lineEditStreet1->setText(address.street1());
    ui->lineEditStreet2->setText(address.street2());
    ui->lineEditPostalCode->setText(address.postalCode());
    QString countryCode = address.countryCode();
    QString countryName = CountryManager::instance()->countryName(countryCode);
    if (!address.countryCode().isEmpty()) {
        ui->comboBoxCountryCode->setCurrentText(countryName);
    }
}
//----------------------------------------------------------
bool WidgetAddress::isAddressComplete() const
{
    return !ui->lineEditLabel->text().isEmpty()
            && !ui->comboBoxCountryCode->currentText().isEmpty();
}
//----------------------------------------------------------
void WidgetAddress::clear()
{
    ui->lineEditLabel->clear();
    ui->lineEditCity->clear();
    ui->lineEditName->clear();
    ui->lineEditState->clear();
    ui->lineEditStreet1->clear();
    ui->lineEditStreet2->clear();
    ui->lineEditPostalCode->clear();
    ui->comboBoxCountryCode->setCurrentIndex(0);
}
//----------------------------------------------------------
