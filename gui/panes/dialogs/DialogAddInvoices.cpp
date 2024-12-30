#include <qmessagebox.h>
#include <qfiledialog.h>
#include <QtCore/qsettings.h>

#include "../common/countries/CountryManager.h"
#include "../common/currencies/CurrencyRateManager.h"

#include "model/CustomerManager.h"

#include "DialogAddInvoices.h"
#include "ui_DialogAddInvoices.h"

//----------------------------------------------------------
DialogAddInvoices::DialogAddInvoices(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddInvoices)
{
    ui->setupUi(this);
    ui->tableWidgetInvoices->setColumnCount(11);
    ui->tableWidgetInvoices->setHorizontalHeaderLabels(
                QStringList({tr("Date"), tr("Titre"), tr("Compte 6"),
                tr("Compte fournisseur"), tr("Montant TTC"),
                tr("Monnaie"), tr("TVA"), tr("Monnaie TVA"),
                tr("Pays TVA"), tr("Montant TTC orig"),
                tr("Monnaie orig")}));
    connect(ui->buttonBrowse,
            &QPushButton::clicked,
            this,
            &DialogAddInvoices::browseFilePaths);
    // TODO delegate
}
//----------------------------------------------------------
DialogAddInvoices::~DialogAddInvoices()
{
    delete ui;
}
//----------------------------------------------------------
QStringList DialogAddInvoices::absFileNames() const
{
    QStringList absFileNames;
    int nFiles = ui->listWidgetFiles->model()->rowCount();
    for (int i=0; i<nFiles; ++i) {
        absFileNames << ui->listWidgetFiles->item(i)->data(
                            Qt::DisplayRole).toString();
    }
    return absFileNames;
}
//----------------------------------------------------------
QList<PurchaseInvoiceInfo>
DialogAddInvoices::getPurchaseInvoiceInfos() const
{
    QList<PurchaseInvoiceInfo> invoiceInfos;
    int nRows = ui->tableWidgetInvoices->rowCount();
    for (int i=0; i<nRows; ++i) {
        PurchaseInvoiceInfo infos;
        infos.absFileName = ui->listWidgetFiles->item(i)->text();
        infos.date = ui->tableWidgetInvoices->item(i, 0)->data(
                    Qt::DisplayRole).toDate();
        infos.label = ui->tableWidgetInvoices->item(i, 1)->data(
                    Qt::DisplayRole).toString();
        infos.account = ui->tableWidgetInvoices->item(i, 2)->data(
                    Qt::DisplayRole).toString();
        infos.accountSupplier = ui->tableWidgetInvoices->item(i, 3)->data(
                    Qt::DisplayRole).toString();
        infos.amount = ui->tableWidgetInvoices->item(i, 4)->data(
                    Qt::DisplayRole).toDouble();
        infos.currency = ui->tableWidgetInvoices->item(i, 5)->data(
                    Qt::DisplayRole).toString();
        infos.amountVat = ui->tableWidgetInvoices->item(i, 6)->data(
                    Qt::DisplayRole).toDouble();
        infos.currencyVat = ui->tableWidgetInvoices->item(i, 7)->data(
                    Qt::DisplayRole).toString();
        infos.vatCountryName = ui->tableWidgetInvoices->item(i, 8)->data(
                    Qt::DisplayRole).toString();
        invoiceInfos << infos;
    }
    return invoiceInfos;
}
//----------------------------------------------------------
void DialogAddInvoices::clear()
{
    ui->listWidgetFiles->clear();
    ui->tableWidgetInvoices->clearContents();
    ui->tableWidgetInvoices->setRowCount(0);
}
//----------------------------------------------------------
void DialogAddInvoices::browseFilePaths()
{
    QSettings settings;
    QString settingKey = "DialogAddInvoicePurchase__browseFilePaths_"
            + CustomerManager::instance()->getSelectedCustomerId();
    QString lastDirPath = settings.value(
                settingKey, QDir().absolutePath()).toString();
    QStringList filePaths = QFileDialog::getOpenFileNames(
                this, tr("Choisisez un ou plusieurs fichiers"),
                lastDirPath);
    if (filePaths.size() > 0) {
        ui->listWidgetFiles->clear();
        ui->listWidgetFiles->addItems(filePaths);
        settings.setValue(settingKey, QFileInfo(filePaths[0]).path());
        ui->tableWidgetInvoices->clearContents();
        ui->tableWidgetInvoices->setRowCount(filePaths.size());
        for (int i=0; i<filePaths.size(); ++i) {
            QFileInfo fileInfo(filePaths[i]);
            QString baseName;
            QStringList fileNameEl = fileInfo.fileName().split(".");
            fileNameEl.takeLast();
            baseName = fileNameEl.join(".");
            QList<QTableWidgetItem *> lineItems;
            if (baseName.contains("__")) {
                Q_ASSERT(!baseName.contains(" "));
                QStringList elements = baseName.split("__");
                QDate date = QDate::fromString(elements[0], "yyyy-MM-dd");
                if (!date.isValid()) {
                    date = QDate(2000, 1, 1);
                }
                QTableWidgetItem *currentItem = new QTableWidgetItem();
                currentItem->setData(Qt::DisplayRole, date);
                lineItems << currentItem;
                QString label;
                QString account;
                QString accountSupplier;
                double amountConv = 0.;
                QString currencyConv;
                double amountVat = 0.;
                QString vatCurrency;
                QString countryNameVat;
                double amount = 0.;
                QString currency;

                int k = 0;
                for (auto element = elements.begin();
                     element != elements.end(); ++element) {
                    bool isNum = false;
                    element->toInt(&isNum);
                    ++k;
                    if (isNum && element->startsWith("6")) {
                        account = *element;
                        int indLabel = k;
                        int indSupplier = k+1;
                        if (indLabel < elements.size())
                        {
                            label = elements[indLabel].replace("-", " ");
                        }
                        if (indSupplier < elements.size())
                        {
                            accountSupplier = elements[indSupplier].replace("-", " ");
                        }
                        break;
                    }
                }
                if (elements.last().size() > 3) {
                    currency = elements.last().right(3);
                    QString amountStr = elements.last().left(elements.last().size()-3);
                    bool isNum = false;
                    amount = amountStr.toDouble(&isNum);
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
                    vatCurrency = lastLast.right(3);
                    QString countryCode = lastLast.split("-TVA")[0];
                    if (countryCode.size() == 2) {
                        countryNameVat
                                = CountryManager::instance()
                                ->countryName(countryCode);
                    }
                    QString amountStr = lastLast.split("-TVA-")[1];
                    amountStr = amountStr.left(amountStr.size()-3);
                    bool isNum = false;
                    amountVat = amountStr.toDouble(&isNum);
                    if (!isNum) {
                        amountVat = 0.;
                    }
                }
                QStringList okCurrencies = {"USD", "EUR", "GBP"};
                if (okCurrencies.contains(currency)) {
                    currencyConv = currency;
                    amountConv = amount;
                } else {
                    QDate convDate = date;
                    if (convDate == QDate(2000, 1, 1)) {
                        convDate = QDate::currentDate();
                    }
                    double rate = CurrencyRateManager::instance()->rate(
                                currency,
                                CustomerManager::instance()->getSelectedCustomerCurrency(),
                                convDate);
                    amountConv = amount * rate;
                    currencyConv = CustomerManager::instance()->getSelectedCustomerCurrency();
                }
                if (currency != currencyConv) {
                    label += " " + QString::number(amount) + " " + currency.toLower();
                }
                // TODO convert non-EUR / USD currency + mark original amount somewhere
                lineItems << new QTableWidgetItem(label);
                lineItems << new QTableWidgetItem(account);
                lineItems << new QTableWidgetItem(accountSupplier);
                currentItem = new QTableWidgetItem();
                currentItem->setData(Qt::DisplayRole, amountConv);
                lineItems << currentItem;
                lineItems << new QTableWidgetItem(currencyConv);
                currentItem = new QTableWidgetItem();
                currentItem->setData(Qt::DisplayRole, amountVat);
                lineItems << currentItem;
                lineItems << new QTableWidgetItem(vatCurrency);
                lineItems << new QTableWidgetItem(countryNameVat);
                currentItem = new QTableWidgetItem();
                currentItem->setData(Qt::DisplayRole, amount);
                lineItems << currentItem;
                lineItems << new QTableWidgetItem(currency);
                for (int j=0; j<lineItems.size(); ++j) {
                    ui->tableWidgetInvoices->setItem(
                                i, j, lineItems[j]);
                }
            }
        }
    }
}
//----------------------------------------------------------
void DialogAddInvoices::accept()
{
    int nRows = ui->tableWidgetInvoices->rowCount();
    for (int i=0; i<nRows; ++i) {
        if (ui->tableWidgetInvoices->item(i, 0)->data(
                    Qt::DisplayRole).toDate() == QDate(2000, 1, 1)) {
            QMessageBox::warning(this,
                                 tr("Erreur"),
                                 tr("Vous devez saisir une date pour chaque facture."));
            return;
        } else if (ui->tableWidgetInvoices->item(i, 1)->data(
                    Qt::DisplayRole).toString().isEmpty()) {
            QMessageBox::warning(this,
                                 tr("Erreur"),
                                 tr("Vous devez saisir un titre pour chaque facture."));
            return;
        } else if (ui->tableWidgetInvoices->item(i, 2)->data(
                    Qt::DisplayRole).toString().isEmpty()) {
            QMessageBox::warning(this,
                                 tr("Erreur"),
                                 tr("Vous devez saisir un compte 6 pour chaque facture."));
            return;
        } else if (ui->tableWidgetInvoices->item(i, 3)->data(
                    Qt::DisplayRole).toString().isEmpty()) {
            QMessageBox::warning(this,
                                 tr("Erreur"),
                                 tr("Vous devez saisir un compte fournisseur pour chaque facture."));
            return;
        } else if (qAbs(ui->tableWidgetInvoices->item(i, 4)->data(
                    Qt::DisplayRole).toDouble()) < 0.001) {
            QMessageBox::warning(this,
                                 tr("Erreur"),
                                 tr("Vous devez saisir un montant non nul pour chaque facture."));
            return;
        }
    }
    QDialog::accept();
}
//----------------------------------------------------------
void DialogAddInvoices::reject()
{
    clear();
    QDialog::reject();
}
//----------------------------------------------------------
