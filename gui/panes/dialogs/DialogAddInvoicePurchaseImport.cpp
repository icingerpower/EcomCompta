#include <qmessagebox.h>
#include <qfiledialog.h>
#include <QtCore/qsettings.h>

#include "../common/countries/CountryManager.h"

#include "model/orderimporters/ShippingAddressesManager.h"
#include "model/CustomerManager.h"

#include "DialogAddInvoicePurchaseImport.h"
#include "ui_DialogAddInvoicePurchaseImport.h"

//----------------------------------------------------------
DialogAddInvoicePurchaseImport::DialogAddInvoicePurchaseImport(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddInvoicePurchaseImport)
{
    ui->setupUi(this);
    ui->comboVatCountry->addItems(
                *CountryManager::instance()->countriesNamesUEfrom2020());
    auto countryNames = CountryManager::instance()->countryNamesSorted();
    int indChina = countryNames->indexOf(CountryManager::CHINA);
    int indFrance = countryNames->indexOf(CountryManager::FRANCE);
    ui->comboBoxCountryFrom->addItems(
                *CountryManager::instance()->countryNames());
    ui->comboBoxCountryFrom->setCurrentIndex(indChina);
    ui->comboBoxCountryTo->addItems(
                *CountryManager::instance()->countryNames());
    ui->comboBoxCountryTo->setCurrentIndex(indFrance);
    connect(ui->buttonBrowse,
            &QPushButton::clicked,
            this,
            &DialogAddInvoicePurchaseImport::browseFilePath);
}
//----------------------------------------------------------
DialogAddInvoicePurchaseImport::~DialogAddInvoicePurchaseImport()
{
    delete ui;
}
//----------------------------------------------------------
QString DialogAddInvoicePurchaseImport::absFileName() const
{
    QString filePath = ui->lineEditFilePath->text();
    return filePath;

}
//----------------------------------------------------------
ImportInvoiceInfo DialogAddInvoicePurchaseImport::getImportInvoiceInfo() const
{
    ImportInvoiceInfo infos;
    infos.date = ui->dateEdit->date();
    infos.label = ui->lineEditLabel->text();
    infos.amount = ui->spinBoxAmount->value();
    infos.accountOrig6 = ui->lineEditAccount->text();
    QString countryNameFrom = ui->comboBoxCountryFrom->currentText();
    QString countryNameTo = ui->comboBoxCountryTo->currentText();
    infos.countryCodeFrom = CountryManager::instance()->countryCode(countryNameFrom);
    infos.countryCodeTo = CountryManager::instance()->countryCode(countryNameTo);
    infos.currency = ui->comboCurrency->currentText();
    infos.amountVat = ui->spinBoxVat->value();
    infos.vatCountryName = ui->comboVatCountry->currentText();
    infos.absFileName = ui->lineEditFilePath->text();
    infos.currencyVat = ui->comboCurrencyVat->currentText();
    return infos;
}
//----------------------------------------------------------
void DialogAddInvoicePurchaseImport::clear()
{
    ui->lineEditFilePath->clear();
    ui->dateEdit->setDate(QDate(2000, 1, 1));
    ui->lineEditLabel->clear();
    ui->lineEditAccount->clear();
    ui->comboBoxCountryFrom->setCurrentIndex(0);
    ui->comboBoxCountryTo->setCurrentIndex(0);
    ui->spinBoxVat->setValue(0.);
    ui->spinBoxAmount->setValue(0.);
    ui->comboCurrency->setCurrentIndex(0);
    ui->comboCurrencyVat->setCurrentIndex(0);
    ui->comboVatCountry->setCurrentText(
                ShippingAddressesManager::instance()->companyCountryName());
}
//----------------------------------------------------------
bool DialogAddInvoicePurchaseImport::wasAccepted() const
{
    return m_wasAccepted;
}
//----------------------------------------------------------
void DialogAddInvoicePurchaseImport::accept()
{
    double amount = ui->spinBoxAmount->value();
    double vat = ui->spinBoxVat->value();
    if (ui->lineEditLabel->text().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Vous devez sélectionner un fichier."));
    } else if (ui->dateEdit->date() == QDate(2000, 1, 1)) {
        QMessageBox::warning(this,
                             tr("Date"),
                             tr("Il faut saisir une date."));
    } else if (ui->lineEditLabel->text().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir un label."));
    } else if (ui->lineEditAccount->text().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir un compte 6."));
    } else if (qAbs(ui->spinBoxAmount->value()) < 0.001) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir un montant TTC non nul."));
    } else if (4 * amount * vat < 0.) {
        auto reply = QMessageBox::question(
                    this,
                    "Montants",
                    "La TVA et le montant total ne sont pas tous les deux négatifs ou positifs. Êtes-vous sûr?",
                    QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            m_wasAccepted = true;
            QDialog::accept();
        }
    } else {
        m_wasAccepted = true;
        QDialog::accept();
    }
}
//----------------------------------------------------------
void DialogAddInvoicePurchaseImport::reject()
{
    clear();
    m_wasAccepted = false;
    QDialog::reject();
}
//----------------------------------------------------------
void DialogAddInvoicePurchaseImport::browseFilePath()
{
    QSettings settings;
    QString settingKey = "DialogAddInvoicePurchase__browseFilePath_"
            + CustomerManager::instance()->getSelectedCustomerId();
    QString lastDirPath = settings.value(
                settingKey, QDir().absolutePath()).toString();
    QString filePath = QFileDialog::getOpenFileName(
                this, tr("Choisir un fichier"),
                lastDirPath);
    if (!filePath.isEmpty()) {
        settings.setValue(settingKey, QFileInfo(filePath).path());
        QFileInfo fileInfo(filePath);
        QString baseName;
        QStringList fileNameEl = fileInfo.fileName().split(".");
        fileNameEl.takeLast();
        baseName = fileNameEl.join(".");
        if (baseName.contains("__")) {
            QStringList elements = baseName.split("__");
            QDate date = QDate::fromString(elements[0], "yyyy-MM-dd");
            if (date.isValid()) {
                ui->dateEdit->setDate(date);
            }
            int i = 0;
            for (auto element : elements) {
                bool isNum = false;
                element.toInt(&isNum);
                if (isNum && element.startsWith("6")) {
                    ui->lineEditAccount->setText(element);
                    int indCountryFrom = i+1;
                    int indCountryTo = i+2;
                    int indLabel = i+3;
                    if (indCountryFrom < elements.size()) {
                        ui->comboBoxCountryFrom->setCurrentText(elements[indCountryFrom]);
                    }
                    if (indCountryTo < elements.size()) {
                        ui->comboBoxCountryTo->setCurrentText(elements[indCountryTo]);
                    }
                    if (indLabel < elements.size()) {
                        ui->lineEditLabel->setText(
                                    elements[indLabel].replace("-", " "));
                    }
                    break;
                }
                ++i;
            }
            if (elements.last().size() > 3) {
                QString currency = elements.last().right(3);
                ui->comboCurrency->setCurrentText(currency);
                QString amountStr = elements.last().left(elements.last().size()-3);
                bool isNum = false;
                double amount = amountStr.toDouble(&isNum);
                if (isNum) {
                    ui->spinBoxAmount->setValue(amount);
                }
            }
            QString lastLast = elements[elements.size()-2];
            if (lastLast.contains("-TVA-")) {
                QString vatCurrency = lastLast.right(3);
                ui->comboCurrencyVat->setCurrentText(vatCurrency);
                QString countryCode = lastLast.split("-TVA")[0];
                if (countryCode.size() == 2) {
                    QString countryName
                            = CountryManager::instance()
                            ->countryName(countryCode);
                    ui->comboVatCountry->setCurrentText(countryName);
                }
                QString amountStr = lastLast.split("-TVA-")[1];
                amountStr = amountStr.left(amountStr.size()-3);
                bool isNum = false;
                double amount = amountStr.toDouble(&isNum);
                if (isNum) {
                    ui->spinBoxVat->setValue(amount);
                }
            }
        }
        ui->lineEditFilePath->setText(filePath);
    }
}
//----------------------------------------------------------
