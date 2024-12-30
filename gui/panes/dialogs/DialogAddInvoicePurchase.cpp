#include <qmessagebox.h>
#include <qfiledialog.h>
#include <QtCore/qsettings.h>

#include "../common/countries/CountryManager.h"

#include "model/CustomerManager.h"
#include "model/orderimporters/ShippingAddressesManager.h"
#include "DialogAddInvoicePurchase.h"
#include "ui_DialogAddInvoicePurchase.h"

//----------------------------------------------------------
DialogAddInvoicePurchase::DialogAddInvoicePurchase(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddInvoicePurchase)
{
    ui->setupUi(this);
    ui->comboVatCountry->addItems(
                *CountryManager::instance()->countriesNamesUEfrom2020());
    connect(ui->buttonBrowse,
            &QPushButton::clicked,
            this,
            &DialogAddInvoicePurchase::browseFilePath);
}
//----------------------------------------------------------
DialogAddInvoicePurchase::~DialogAddInvoicePurchase()
{
    delete ui;
}
//----------------------------------------------------------
QString DialogAddInvoicePurchase::absFileName() const
{
    QString filePath = ui->lineEditFilePath->text();
    return filePath;
}
//----------------------------------------------------------
PurchaseInvoiceInfo DialogAddInvoicePurchase::getPurchaseInvoiceInfo() const
{
    PurchaseInvoiceInfo infos;
    infos.date = ui->dateEdit->date();
    infos.label = ui->lineEditLabel->text();
    infos.amount = ui->spinBoxAmount->value();
    infos.account = ui->lineEditAccount->text();
    infos.currency = ui->comboCurrency->currentText();
    infos.accountSupplier = ui->lineEditAccountSupplier->text();
    infos.amountVat = ui->spinBoxVat->value();
    infos.vatCountryName = ui->comboVatCountry->currentText();
    infos.absFileName = ui->lineEditFilePath->text();
    infos.currencyVat = ui->comboCurrencyVat->currentText();
    return infos;
}
//----------------------------------------------------------
void DialogAddInvoicePurchase::clear()
{
    ui->lineEditFilePath->clear();
    ui->dateEdit->setDate(QDate(2000, 1, 1));
    ui->lineEditLabel->clear();
    ui->lineEditAccount->clear();
    ui->lineEditAccountSupplier->clear();
    ui->spinBoxVat->setValue(0.);
    ui->spinBoxAmount->setValue(0.);
    ui->comboCurrency->setCurrentIndex(0);
    ui->comboCurrencyVat->setCurrentIndex(0);
    ui->comboVatCountry->setCurrentText(
                ShippingAddressesManager::instance()->companyCountryName());
}
//----------------------------------------------------------
void DialogAddInvoicePurchase::accept()
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
    } else if (ui->lineEditAccountSupplier->text().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir un compte fournisseur."));
    } else if (qAbs(ui->spinBoxAmount->value()) < 0.001) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir un montant TTC non nul."));
    } else if (4 * amount * vat < 0.) {
    } else {
        if (4 * amount * vat < 0.) {
            auto reply = QMessageBox::question(
                        this,
                        "Montants",
                        "La TVA et le montant total ne sont pas tous les deux négatifs ou positifs. Êtes-vous sûr?",
                        QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::No) {
                return;
            }
        }
        const auto &date = ui->dateEdit->date();
        const auto currentDate = QDate::currentDate();
        if (date.daysTo(currentDate) > 60 && date.year() < currentDate.year())
        {
            auto reply = QMessageBox::question(
                        this,
                        "Date",
                        "Vous avez saisie l'année dernière. Êtes-vous sûr?",
                        QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::No) {
                return;
            }
        }
        QDialog::accept();
    }
}
//----------------------------------------------------------
void DialogAddInvoicePurchase::browseFilePath()
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
            Q_ASSERT(!baseName.contains(" "));
            QStringList elements = baseName.split("__");
            QDate date = QDate::fromString(elements[0], "yyyy-MM-dd");
            if (date.isValid()) {
                ui->dateEdit->setDate(date);
            }
            int i = 0;
            for (const auto &element : elements) {
                bool isNum = false;
                element.toInt(&isNum);
                ++i;
                if (isNum && element.startsWith("6")) {
                    ui->lineEditAccount->setText(element);
                    int indLabel = i;
                    int indSupplier = i+1;
                    if (indLabel < elements.size()) {
                        ui->lineEditLabel->setText(
                                    elements[indLabel].replace("-", " "));
                    }
                    if (indSupplier < elements.size()) {
                        ui->lineEditAccountSupplier->setText(
                                    elements[indSupplier].replace("-", " "));
                    }
                    break;
                }
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
            Q_ASSERT(!lastLast.contains(","));
            Q_ASSERT(!lastLast.contains("--T"));
            Q_ASSERT(!baseName.contains(" "));
            if (lastLast.contains("--"))
            {
                Q_ASSERT(baseName.contains("_-"));
            }
            if (baseName.contains("_-"))
            {
                Q_ASSERT(!baseName.contains("TVA") || baseName.contains("--"));
            }
            Q_ASSERT(!lastLast.contains("FR-") || lastLast.contains("FR-TVA"));
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

