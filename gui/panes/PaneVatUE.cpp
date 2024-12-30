#include <qprogressdialog.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <QtCore/qset.h>
#include <QPageSize>
#include <QtCore/qsettings.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <qtextdocument.h>
#include <QtPrintSupport/qprinter.h>
#include <qpainter.h>
#include <qpdfwriter.h>
#include <qfiledialog.h>

#include "../common/countries/CountryManager.h"
#include "../common/currencies/ExceptionCurrencyRate.h"

#include "PaneVatUE.h"
#include "ui_PaneVatUE.h"
#include "dialogs/DialogDiffVatAmazonUe.h"
#include "model/orderimporters/ModelDiffAmazonUE.h"
#include "model/orderimporters/OrderManager.h"
#include "model/orderimporters/RefundManager.h"
#include "model/SettingManager.h"
#include "model/CustomerManager.h"
#include "model/orderimporters/VatOrdersModel.h"
#include "model/orderimporters/ImporterYearsManager.h"
#include "model/orderimporters/ModelStockDeported.h"
#include "model/bookkeeping/ExceptionAccountSaleMissing.h"
#include "dialogs/DialogDisplayOrdersMissingReports.h"
#include "dialogs/DialogDisplayUncompleteOrders.h"

//----------------------------------------------------------
PaneVatUE::PaneVatUE(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneVatUE)
{
    ui->setupUi(this);
    m_orderManagerFilter = nullptr;
    ui->treeViewVat->setModel(VatOrdersModel::instance());
    ui->treeViewVat->header()->resizeSection(0, 200);
    ui->comboBoxYear->setModel(ImporterYearsManager::instance());
    ui->widgetDetails->hide();
    _connectSlots();
    _loadSettings();
    erase();
}
//----------------------------------------------------------
PaneVatUE::~PaneVatUE()
{
    delete ui;
    m_orderManagerFilter->deleteLater();
}
//----------------------------------------------------------
void PaneVatUE::onCustomerSelectedChanged(const QString &customerId)
{
    _loadSettings();
}
//----------------------------------------------------------
void PaneVatUE::_connectSlots()
{
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            this,
            &PaneVatUE::onCustomerSelectedChanged);
    connect(ui->buttonCompute,
            &QPushButton::clicked,
            this,
            &PaneVatUE::compute);
    connect(ui->buttonComputeSelection,
            &QPushButton::clicked,
            this,
            &PaneVatUE::computeSelectedPeriod);
    connect(ui->buttonErase,
            &QPushButton::clicked,
            this,
            &PaneVatUE::erase);
    connect(ui->buttonDownloadCsv,
            &QPushButton::clicked,
            this,
            &PaneVatUE::downloadCsvOrders);
    connect(ui->buttonDownloadVatCsv,
            &QPushButton::clicked,
            this,
            &PaneVatUE::downloadCsvVat);
    connect(ui->buttonDownloadPdf,
            &QPushButton::clicked,
            this,
            &PaneVatUE::downloadPdfReport);
    connect(ui->buttonDiffAmazon,
            &QPushButton::clicked,
            this,
            &PaneVatUE::compareDifferencesWithAmazon);
    connect(ui->buttonBrowseInvoicesDir,
            &QPushButton::clicked,
            this,
            &PaneVatUE::browseInvoiceDir);
    connect(ui->buttonBrowseBookKeepingDir,
            &QPushButton::clicked,
            this,
            &PaneVatUE::browseBookKeepingDir);
    connect(VatOrdersModel::instance(),
            &VatOrdersModel::orderWithUncompleteReports,
            this,
            &PaneVatUE::displayOrderWithUncompleteReports);
    connect(VatOrdersModel::instance(),
            &VatOrdersModel::shipmentsNotCompletelyLoaded,
            this,
            &PaneVatUE::displayUncompleteShipments);
    connect(VatOrdersModel::instance(),
            &VatOrdersModel::skusWithNoValuesFound,
            this,
            &PaneVatUE::displaySkusWithNoValuesFound);
    connect(ui->checkBoxGenerateInvoices,
            &QCheckBox::clicked,
            [this](bool checked){
        ui->lineEditInvoicesDir->setEnabled(checked);
        ui->buttonBrowseInvoicesDir->setEnabled(checked);
        QSettings settings;
        settings.setValue(settingKeyCheckInvoice(), checked);
    });
    connect(ui->checkBoxGenerateAccountingEntries,
            &QCheckBox::clicked,
            [this](bool checked){
        ui->lineEditBookKeepingDir->setEnabled(checked);
        ui->buttonBrowseBookKeepingDir->setEnabled(checked);
        QSettings settings;
        settings.setValue(settingKeyCheckBookKeeping(), checked);
    });
    /*
    connect(ui->buttonExpandOrders,
        &QPushButton::clicked,
        this,
        &PaneVatUE::expandOrders);
        //*/
}
//----------------------------------------------------------
void PaneVatUE::compute()
{
    QString yearString = ui->comboBoxYear->currentText();
    if (!yearString.isEmpty()) {
        setCursor(Qt::WaitCursor);
        /*
        QProgressDialog progressDialog;
        progressDialog.setRange(0, 100);
        progressDialog.setValue(0);
        progressDialog.setLabelText(tr("Chargement en cours..."));
        progressDialog.setAutoClose(true);
        connect(VatOrdersModel::instance(),
                &VatOrdersModel::progressed,
                &progressDialog,
                &QProgressDialog::setValue);
        progressDialog.show();
        //*/
        QSettings settings;
        bool genInvoices = ui->checkBoxGenerateInvoices->isChecked();
        QString dirInvoices;
        if (genInvoices) {
            dirInvoices = ui->lineEditInvoicesDir->text();
        }
        bool genBookKeeping = ui->checkBoxGenerateAccountingEntries->isChecked();
        QString dirBookKeeping;
        if (genBookKeeping) {
            dirBookKeeping = ui->lineEditBookKeepingDir->text();
        }
        try {
            VatOrdersModel::instance()->computeVat(
                        yearString.toInt(),
                        [](const Shipment *) {},
            dirBookKeeping,
            dirInvoices);
            //disconnect(VatOrdersModel::instance(),
            //&VatOrdersModel::progressed,
            //&progressDialog,
            //&QProgressDialog::setValue);
            if (ui->tabOrders->orderManager() == nullptr) {
                //ui->treeViewOrders->setModel(VatTableModelUE::instance()->orderManager());
                //ui->treeViewOrders->header()->resizeSection(0, 200);
                m_refundManager = new RefundManager(VatOrdersModel::instance()->orderManager());
                ui->tabOrders->init(VatOrdersModel::instance()->orderManager(), m_refundManager);
                ui->tabRefunds->init(VatOrdersModel::instance()->orderManager(), m_refundManager);
            } else if (ui->tabOrders->orderManager() != VatOrdersModel::instance()->orderManager()) {
                //ui->treeViewOrders->model()->deleteLater();
                m_refundManager->deleteLater();
                //ui->treeViewOrders->setModel(VatTableModelUE::instance()->orderManager());
                //ui->treeViewOrders->header()->resizeSection(0, 200);
                m_refundManager = new RefundManager(VatOrdersModel::instance()->orderManager());
                ui->tabOrders->init(VatOrdersModel::instance()->orderManager(), m_refundManager);
                ui->tabRefunds->init(VatOrdersModel::instance()->orderManager(), m_refundManager);
            }
            ui->widgetDetails->show();
            ui->treeViewVat->expandToDepth(0);
            ui->tabOrders->expandAllOrders();
            ui->tabRefunds->expandAll();
        } catch (const ExceptionCurrencyRate &exception){
            QString error = tr("Impossible d’ouvrir") + ": " + exception.url();
            error += ". " + tr("Vérifier votre connection internet et votre parefeu.");
            QMessageBox::warning(
                        this,
                        tr("Problème avec internet"),
                        error);
        } catch (const ExceptionAccountSaleMissing &exception){
            QString error = tr("Les comptes de vente n’ont pas été configuré pour l’ensemble suivant")
                    + ": " + exception.accounts();
            QMessageBox::warning(
                        this,
                        tr("Comptes de vente manquant"),
                        error);
        }
        setCursor(Qt::ArrowCursor);
    }
}
//----------------------------------------------------------
void PaneVatUE::erase()
{
      ui->tableWidgetComputed->clear();
      ui->tableWidgetComputed->setRowCount(0);
      ui->tableWidgetComputed->setColumnCount(2);
      //static QStringList labels = {VatTableModelUE::titleTotalTaxed,
      //VatTableModelUE::titleTotalUntaxed,
      //VatTableModelUE::titleTaxes};
      //ui->tableWidgetComputed->setVerticalHeaderLabels(labels);
      ui->tableWidgetComputed->setHorizontalHeaderLabels({tr("Titre"), tr("Montant")});
      if (m_orderManagerFilter != nullptr) {
        //ui->treeViewOrders->setModel(nullptr);
        ui->tabOrders->clear();
        m_orderManagerFilter->deleteLater();
        m_refundManager->deleteLater();
      }
}
//----------------------------------------------------------
void PaneVatUE::displayOrderWithUncompleteReports(
            const QMap<QString, QList<QStringList> > &orders)
{
      DialogDisplayOrdersMissingReports *dialog = new DialogDisplayOrdersMissingReports();
      dialog->setOrderInfos(orders);
      connect(dialog,
              &DialogDisplayOrdersMissingReports::accepted,
              [dialog](){
        dialog->deleteLater();
      });
      dialog->show();
}
//----------------------------------------------------------
void PaneVatUE::displayUncompleteShipments(
        const QMultiMap<QDateTime, Shipment *> &shipmentsAndRefunds)
{
      DialogDisplayUncompleteOrders *dialog = new DialogDisplayUncompleteOrders();
      dialog->setShipmentsRefunds(shipmentsAndRefunds);
      if (dialog->nRows() > 0) {
          connect(dialog,
                  &DialogDisplayUncompleteOrders::accepted,
                  [dialog](){
              dialog->deleteLater();
          });
          dialog->show();
      } else {
          dialog->deleteLater();
      }
}
//----------------------------------------------------------
void PaneVatUE::displaySkusWithNoValuesFound(const QStringList &)
{
    auto computingType = ModelStockDeported::instance()->computingType();
    if (computingType == StockDeportedComputing::TableValues) {
        QMessageBox::warning(this,
                             tr("SKUs sans valeurs pour le calcul du stock déportés"),
                             tr("Plusieurs SKUS déportés sans valeur ont été trouvés."
                                " Vous pouvez renseigner les valeurs dans Paramètres > Stock déportés"));
    }
}
//----------------------------------------------------------
void PaneVatUE::compareDifferencesWithAmazon()
{
      bool hasYear = true;
      int yearInt = ui->comboBoxYear->currentText().toInt(&hasYear);
      if (hasYear) {
        static ModelDiffAmazonUE *modelDiffUe
                    = new ModelDiffAmazonUE(this);
        modelDiffUe->compute(VatOrdersModel::instance()->orderManager(), yearInt);
        if (modelDiffUe->rowCount() == 0) {
          QMessageBox::information(
                            this,
                            tr("Pas de différence trouvé"),
                            tr("La TVA calculée est la même que celle d'amazon"));
        } else {
          DialogDiffVatAmazonUe *dialog
                      = new DialogDiffVatAmazonUe(this);
          dialog->setModelDiffAmazonUE(modelDiffUe);
          connect(dialog,
                  &DialogDiffVatAmazonUe::accepted,
                  [dialog](){
            dialog->deleteLater();
          });
          dialog->show();
        }
      }
}
//----------------------------------------------------------
void PaneVatUE::_loadSettings()
{
    QSettings settings;
    QString keyInvoice = settingKeyDirInvoice();
    QString lastDirPathInvoice = settings.value(keyInvoice).toString();
    if (!lastDirPathInvoice.isEmpty()) {
        ui->lineEditInvoicesDir->setText(lastDirPathInvoice);
    }
    QString keyBookKeeping = settingKeyDirBookKeeping();
    QString lastDirPathBookKeeping
            = settings.value(keyBookKeeping).toString();
    if (!lastDirPathBookKeeping.isEmpty()) {
        ui->lineEditBookKeepingDir->setText(lastDirPathBookKeeping);
    }
    bool genInvoices = settings.value(
                settingKeyCheckInvoice(), false).toBool();
    if (!genInvoices) {
        ui->buttonBrowseInvoicesDir->setEnabled(false);
        ui->lineEditInvoicesDir->setEnabled(false);
    }
    ui->checkBoxGenerateInvoices
            ->setChecked(genInvoices);
    bool genBookKeeping = settings.value(
                settingKeyCheckBookKeeping(), false).toBool();
    if (!genBookKeeping) {
        ui->buttonBrowseBookKeepingDir->setEnabled(false);
        ui->lineEditBookKeepingDir->setEnabled(false);
    }
    ui->checkBoxGenerateAccountingEntries
            ->setChecked(genBookKeeping);
}
//----------------------------------------------------------
void PaneVatUE::browseInvoiceDir()
{
    QSettings settings;
    QString keyInvoice = settingKeyDirInvoice();
    QString lastDirPath = settings.value(
                keyInvoice, QDir().absolutePath()).toString();
    QString directory = QFileDialog::getExistingDirectory(
                this, tr("Choisir un répertoire"),
                lastDirPath);
    if (!directory.isEmpty()) {
        settings.setValue(keyInvoice, directory);
    }
    ui->lineEditInvoicesDir->setText(directory);
}
//----------------------------------------------------------
void PaneVatUE::browseBookKeepingDir()
{
    QSettings settings;
    QString keyBookKeeping = settingKeyDirBookKeeping();
    QString lastDirPath = settings.value(
                keyBookKeeping, QDir().absolutePath()).toString();
    QString directory = QFileDialog::getExistingDirectory(
                this, tr("Choisir un répertoire"),
                lastDirPath);
    if (!directory.isEmpty()) {
        settings.setValue(keyBookKeeping, directory);
    }
    ui->lineEditBookKeepingDir->setText(directory);
}
//----------------------------------------------------------
void PaneVatUE::computeSelectedPeriod()
{
    if (_checkTableSelIndexes()) {
        auto selIndexes = ui->treeViewVat->selectionModel()->selectedIndexes();
        erase();
        VatTableNodeVatCountry *item
                = static_cast<VatTableNodeVatCountry *>(
                    selIndexes[0].internalPointer());
        QString vatCountry = item->title().split(" ").last();
        QString vatCountryCode = CountryManager::instance()->countryCode(vatCountry);
        QString vatRegime = item->parent()->title();
        bool allCountry = item->title() == VatOrdersModel::titleTotalTaxed
                || item->title() == VatOrdersModel::titleTotalUntaxed
                || item->title() == VatOrdersModel::titleTaxes;
        QSet<int> months;
        for (auto sel : selIndexes) {
            months << sel.column();
        }
        m_orderManagerFilter
                = VatOrdersModel::instance()->orderManager()->copyFilter(
                    [months, vatCountryCode, vatRegime, allCountry](const Order * order) -> bool {
            for (auto shipment : order->getShipments()) {
                if (months.contains(shipment->getDateTime().date().month())
                        && shipment->getRegimeVat() == vatRegime
                        && (allCountry
                            || shipment->countryVat() == vatCountryCode)) {
                    return true;
                }
            }
            return false;
        },[months, vatCountryCode, vatRegime, allCountry](const Refund * refund) -> bool {
            return months.contains(refund->getDateTime().date().month())
                    && refund->getRegimeVat() == vatRegime
                    && (allCountry
                        || refund->countryVat() == vatCountryCode);
        });
        m_refundManager = new RefundManager(m_orderManagerFilter);
        ui->tabOrders->init(m_orderManagerFilter, m_refundManager);
        ui->tabRefunds->init(m_orderManagerFilter, m_refundManager);
        ui->tabOrders->expandAllOrders();
        ui->tabRefunds->expandAll();
        _generateTableData(selIndexes);
        ui->tableWidgetComputed->horizontalHeader()->resizeSection(0, 280);
        ui->tableWidgetComputed->setRowCount(m_tableVatData.size()-1);
        ui->tableWidgetComputed->setColumnCount(m_tableVatData[0].size());
        ui->tableWidgetComputed->setHorizontalHeaderLabels(m_tableVatData[0]);
        for (int i=1; i<m_tableVatData.size(); ++i) {
            for (int j=0; j<m_tableVatData[0].size(); ++j) {
                auto itemTable = new QTableWidgetItem(m_tableVatData[i][j]);
                itemTable->setData(Qt::BackgroundRole, m_tableLineColors[i-1]);
                ui->tableWidgetComputed->setItem(
                            i-1, j, itemTable);
            }
        }
    }
}
//----------------------------------------------------------
bool PaneVatUE::_checkTableSelIndexes()
{
      auto selIndexes = ui->treeViewVat->selectionModel()->selectedIndexes();
      QSet<int> rows;
      for (auto index : selIndexes) {
        rows << index.row();
      }
      if (rows.size() == 0) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Vous devez sélectionner une ligne et un ou plusieurs mois"));
        return false;
      } else if (rows.size() > 1) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Vous devez sélectionner une seule ligne"));
        return false;
      }
      return true;
}
//----------------------------------------------------------
void PaneVatUE::_generateTableData(const QModelIndexList &indexes)
{
      VatTableNodeVatCountry *item
                  = static_cast<VatTableNodeVatCountry *>(
                        indexes[0].internalPointer());
      m_tableCountryNameVat = item->title().split(" ").last();
      if (VatOrdersModel::titleTotalTaxed.endsWith(m_tableCountryNameVat)) {
        m_tableCountryNameVat.clear();
    }
    m_tableRegime = item->parent()->title();
    QSet<int> months;
    for (auto sel : indexes) {
        months << sel.column();
    }
    m_months = months.toList();
    std::sort(m_months.begin(), m_months.end());
    m_tableLineColors.clear();
    m_tableVatData.clear();
    if (item->parent()->title().contains("OSS") && !item->parent()->title().contains("IOSS")) {
        QList<QList<double>> totals;
        double totalUntaxed = 0.;
        double totalTaxed = 0.;
        double totalTaxes = 0.;
        QMap<QString, double> totalsUntaxedByCountry;
        QMap<QString, double> totalsTaxedByCountry;
        QMap<QString, double> totalsTaxesByCountry;
        QMap<QString, QMap<QString, QMap<QString, double>>> untaxedByCountry;
        QMap<QString, QMap<QString, QMap<QString, double>>> taxedByCountry;
        QMap<QString, QMap<QString, QMap<QString, double>>> taxesByCountry; // TODO add dimension percentage VAT
        int row = 0;
        int nLines = 1;
        QString country;
        for (auto child : item->children()) {
            QString countryTo = child->title().split(" ").last();
            Q_ASSERT(!countryTo.isEmpty());
            if (!totalsTaxedByCountry.contains(countryTo)) {
                totalsUntaxedByCountry[countryTo] = 0.;
                totalsTaxedByCountry[countryTo] = 0.;
                totalsTaxesByCountry[countryTo] = 0.;
                untaxedByCountry[countryTo] = QMap<QString, QMap<QString, double>>();
                taxedByCountry[countryTo] = QMap<QString, QMap<QString, double>>();
                taxesByCountry[countryTo] = QMap<QString, QMap<QString, double>>();
            }
            ++nLines;
            for (auto child2 : child->children()) {
                auto item = static_cast<VatTableNodeVatCountry *>(child2);
                QString countryFrom = item->title().split(" => ")[0].split(" ").last();
                QString taxeRate = item->title().split(" ").last();
                if (!taxedByCountry[countryTo].contains(countryFrom)) {
                    taxedByCountry[countryTo][countryFrom] = QMap<QString, double>();
                    untaxedByCountry[countryTo][countryFrom] = QMap<QString, double>();
                    taxesByCountry[countryTo][countryFrom] = QMap<QString, double>();
                }
                if (!taxedByCountry[countryTo][countryFrom].contains(taxeRate)) {
                    taxedByCountry[countryTo][countryFrom][taxeRate] = 0.;
                    untaxedByCountry[countryTo][countryFrom][taxeRate] = 0.;
                    taxesByCountry[countryTo][countryFrom][taxeRate] = 0.;
                }
                double total = item->total(months);
                totalTaxed += total;
                totalsTaxedByCountry[countryTo] += total;
                taxedByCountry[countryTo][countryFrom][taxeRate] += total;
                if (row % 2 == 0) { /// HT
                    totalsUntaxedByCountry[countryTo] += total;
                    totalUntaxed += total;
                    untaxedByCountry[countryTo][countryFrom][taxeRate] += total;
                    ++nLines;
                } else { /// HT
                    totalsTaxesByCountry[countryTo] += total;
                    totalTaxes += total;
                    taxesByCountry[countryTo][countryFrom][taxeRate] += total;
                }
                ++row;
            }
        }
        //ui->tableWidgetComputed->setRowCount(nLines);
        m_tableVatData << QStringList({tr("Titre"), tr("Montant HT"), tr("TVA"), tr("Montant TTC")});
        m_tableVatData << QStringList();
        m_tableLineColors << SettingManager::instance()->colorBlue();
        m_tableVatData.last() << tr("Ventes totales");
        m_tableVatData.last() << QString::number(totalUntaxed, 'f', 2);
        m_tableVatData.last() << QString::number(totalTaxes, 'f', 2);
        m_tableVatData.last() << QString::number(totalTaxed, 'f', 2);
        int idRow=1;
        for (auto countryTo : taxedByCountry.keys()) {
            m_tableVatData << QStringList();
            m_tableLineColors << SettingManager::instance()->colorLightBlue();
            m_tableVatData.last() << tr("Ventes totales") + " " + countryTo;
            m_tableVatData.last() << QString::number(totalsUntaxedByCountry[countryTo], 'f', 2);
            m_tableVatData.last() << QString::number(totalsTaxesByCountry[countryTo], 'f', 2);
            m_tableVatData.last() << QString::number(totalsTaxedByCountry[countryTo], 'f', 2);
            ++idRow;
            for (auto countryFrom : taxedByCountry[countryTo].keys()) {
                for (auto taxRate : taxedByCountry[countryTo][countryFrom].keys()) {
                    m_tableVatData << QStringList();
                    m_tableLineColors << SettingManager::instance()->colorTurquoise();
                    Q_ASSERT(!countryFrom.isEmpty() && !countryTo.isEmpty());
                    m_tableVatData.last() << tr("Ventes totales") + " " + countryFrom + " => " + countryTo + " " + taxRate;
                    m_tableVatData.last() << QString::number(untaxedByCountry[countryTo][countryFrom][taxRate], 'f', 2);
                    m_tableVatData.last() << QString::number(taxesByCountry[countryTo][countryFrom][taxRate], 'f', 2);
                    m_tableVatData.last() << QString::number(taxedByCountry[countryTo][countryFrom][taxRate], 'f', 2);
                    ++idRow;
                }
            }
        }
    } else {
        QStringList titles;
        QList<double> totals;
        QList<VatTableNodeVatCountry *> items;
        items << item;
        for (auto child : item->children()) {
            items << static_cast<VatTableNodeVatCountry *>(child);
            for (auto child2 : child->children()) {
                items << static_cast<VatTableNodeVatCountry *>(child2);
            }
        }
        for (auto child : items) {
            VatTableNodeVatDetails *childDetails
                    = static_cast<VatTableNodeVatDetails *>(child);
            titles << child->title();
            double total = childDetails->total(months);
            totals << total;
        }
        m_tableVatData << QStringList({tr("Titre"), tr("Montant")});
        for (int i=0; i<titles.size(); ++i) {
            m_tableVatData << QStringList({titles[i], QString::number(totals[i], 'f', 2)});
            if (i%2 == 0) {
                m_tableLineColors << SettingManager::instance()->colorLightBlue();
            } else {
                m_tableLineColors << SettingManager::instance()->colorTurquoise();
            }
        }
    }
}
//----------------------------------------------------------
QString PaneVatUE::settingKeyCheckInvoice() const
{
    QString keyRadioInvoice
            = "PaneVatUE__radioInvoice"
            + CustomerManager::instance()->getSelectedCustomerId();
    return keyRadioInvoice;
}
//----------------------------------------------------------
QString PaneVatUE::settingKeyCheckBookKeeping() const
{
    QString keyRadioBookKeeping
            = "PaneVatUE__radioBookKeeping"
            + CustomerManager::instance()->getSelectedCustomerId();
    return keyRadioBookKeeping;
}
//----------------------------------------------------------
QString PaneVatUE::settingKeyDirInvoice() const
{
    QString keyInvoice = "PaneVatUE__browseInvoiceDir"
            + CustomerManager::instance()->getSelectedCustomerId();
    return keyInvoice;
}
//----------------------------------------------------------
QString PaneVatUE::settingKeyDirBookKeeping() const
{
    QString keyBookKeeping = "PaneVatUE__browseBookKeepingDir"
            + CustomerManager::instance()->getSelectedCustomerId();
    return keyBookKeeping;
}
//----------------------------------------------------------
void PaneVatUE::downloadCsvVat()
{
    QSettings settings;
    QString key = "PaneVatUE_downloadCsvVat";
    QString lastDirPath = settings.value(
                key, QDir().absolutePath()).toString();
    QString filePath = QFileDialog::getSaveFileName(
                this, tr("Choisir un fichier"),
                lastDirPath,
                "CSV (*.csv)");
    if (!filePath.isEmpty()) {
        if (!filePath.toLower().endsWith(".csv")) {
            filePath += ".csv";
        }
        QFileInfo fileInfo(filePath);
        settings.setValue(key, fileInfo.absoluteDir().absolutePath());
        QStringList lines;
        QStringList lineHeaderElements;
        //lineHeaderElements << "";
        for (int col = 0; col < ui->tableWidgetComputed->columnCount(); ++col) {
            lineHeaderElements << ui->tableWidgetComputed->horizontalHeaderItem(
                                      col)->data(
                                      Qt::DisplayRole).toString();
        }
        lines << lineHeaderElements.join(";");
        for (int row = 0; row < ui->tableWidgetComputed->rowCount(); ++row) {
            QStringList lineElements;
            //lineElements << ui->tableWidgetComputed->verticalHeaderItem(
                                //row)->data(
                                //Qt::DisplayRole).toString();
            for (int col = 0; col < ui->tableWidgetComputed->columnCount(); ++col) {
                lineElements << ui->tableWidgetComputed->item(
                                    row, col)->data(
                                    Qt::DisplayRole).toString();
            }
            lines << lineElements.join(";");
        }
        QFile file(filePath);
        if (file.open(QFile::WriteOnly)) {
            QString fileContent = lines.join(SettingManager::instance()->returnLine());
            QTextStream stream(&file);
            stream << fileContent;
            file.close();
        }
    }
}
//----------------------------------------------------------
void PaneVatUE::downloadCsvOrders()
{
    QSettings settings;
    QString key = "PaneVatUE_downloadCsv";
    QString lastDirPath = settings.value(
                key, QDir().absolutePath()).toString();
    QString filePath = QFileDialog::getSaveFileName(
                this, tr("Choisir un fichier"),
                lastDirPath,
                "CSV (*.csv)");
    if (!filePath.isEmpty()) {
        if (!filePath.toLower().endsWith(".csv")) {
            filePath += ".csv";
        }
        QFileInfo fileInfo(filePath);
        settings.setValue(key, fileInfo.absoluteDir().absolutePath());
        m_orderManagerFilter->exportCsv(filePath);
    }
}
//----------------------------------------------------------
void PaneVatUE::downloadPdfReport()
{
    if (_checkTableSelIndexes()) {
        QSettings settings;
        QString key = "PaneVatUE_downloadPdf";
        QString lastDirPath = settings.value(
                    key, QDir().absolutePath()).toString();
        QString filePath = QFileDialog::getSaveFileName(
                    this, tr("Choisir un fichier"),
                    lastDirPath,
                    "PDF (*.pdf)");
        if (!filePath.isEmpty()) {
            if (!filePath.toLower().endsWith(".pdf")) {
                filePath += ".pdf";
            }
            QFileInfo fileInfo(filePath);
            settings.setValue(key, fileInfo.absoluteDir().absolutePath());
            auto indexes = ui->treeViewVat->selectionModel()->selectedIndexes();
            _generateTableData(indexes);
            QTextDocument document;
            QString html;
            html += "<html><body>";
            html += "<h1>" + tr("TVA") + " " + m_tableRegime;
            if (!m_tableCountryNameVat.isEmpty()) {
                html += " " + m_tableCountryNameVat;
            }
            QString yearString = ui->comboBoxYear->currentText();
            QDate begin(yearString.toInt(), m_months.first(), 1);
            QDate end(yearString.toInt(), m_months.last(), 1);
            end = end.addMonths(1).addDays(-1);
            QString formatDate = tr("dd/MM/yyyy");
            html += " " + begin.toString(formatDate) + " => " + end.toString(formatDate);
            html += "</h1><br>";
            html += _createHtmlTable(m_tableVatData, m_tableLineColors, false);
            html += "<br><br><br>";
            html += "<h1>" + QObject::tr("Commandes et remboursements de la période") + "</h1>";
            html += "<br>";
            QList<QStringList> tableOrdersPeriod;
            QList<QBrush> tableOrdersPeriodColors;
            QStringList header;
            for (auto colInfo : colInfos()) {
                header << colInfo.name;
            }
            tableOrdersPeriod << header;
            auto shipmentsAndRefunds
                    = m_orderManagerFilter->getShipmentsAndRefunds(
                        begin, end);
            int i=0;
            QSet<QString> shipRefBeforeIds;
            for (auto shipmentOrRefund : shipmentsAndRefunds) {
                if (shipmentOrRefund->getRegimeVat() == m_tableRegime
                        && (m_tableCountryNameVat.isEmpty()
                            || m_tableCountryNameVat == shipmentOrRefund->getCountryNameVat())) {
                    shipRefBeforeIds << shipmentOrRefund->getId();
                    QStringList lineElements;
                    for (auto colInfo : colInfos()) {
                        lineElements << colInfo.getValue(shipmentOrRefund);
                    }
                    tableOrdersPeriod << lineElements;
                    if (i % 2 == 0) {
                        tableOrdersPeriodColors << SettingManager::instance()->colorLightBlue();
                    } else {
                        tableOrdersPeriodColors << SettingManager::instance()->colorTurquoise();
                    }
                }
                ++i;
            }
            html += _createHtmlTable(tableOrdersPeriod, tableOrdersPeriodColors, true);
            html += "<br><br><br>";
            html += "<h1>" + QObject::tr("Autres commandes des autres déclarations de TVA") + "</h1>";
            html += "<br>";
            QList<QStringList> tableOthers;
            QList<QBrush> tableOthersColors;
            tableOthers << header;
            auto shipRefAll
                    = VatOrdersModel::instance()->orderManager()->getShipmentsAndRefunds(
                        begin, end);
            i=0;
            for (auto shipRef : shipRefAll) {
                if (!shipRefBeforeIds.contains(shipRef->getId())) {
                    QStringList lineElements;
                    for (auto colInfo : colInfos()) {
                        lineElements << colInfo.getValue(shipRef);
                    }
                    tableOthers << lineElements;
                    if (i % 2 == 0) {
                        tableOthersColors << SettingManager::instance()->colorLightBlue();
                    } else {
                        tableOthersColors << SettingManager::instance()->colorTurquoise();
                    }
                    ++i;
                }
            }
            html += _createHtmlTable(tableOthers, tableOthersColors, true);
            html += "</html></body>";
            //QPageSize pagesSize(QPageSize::A1);
            //document.setTextWidth(pagesSize.size());
            QPrinter printer;
            printer.setPageMargins(10.0,10.0,10.0,10.0,printer.Millimeter);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setPageSize(QPrinter::A1);
            auto width = printer.pageRect().width();
            document.setTextWidth(width);
            document.setHtml(html);
            printer.setColorMode(QPrinter::Color);
            printer.setOutputFileName(filePath);
            document.print(&printer);

        }
    }
}
//----------------------------------------------------------
QString PaneVatUE::_createHtmlTable(
        const QList<QStringList> &tableWithHeader,
        const QList<QBrush> &tableLineColors,
        bool textCenter)
{
    QString cellStyle= " style=\"padding:8px;";
    if (textCenter) {
        cellStyle += "text-align:center\"";
    } else {
        cellStyle += "\"";
    }
    QString html = "<table>";
    html += "<tr><th>";
    //html += tableWithHeader[0].join("</th><th style=\"width:100px\">");
    html += tableWithHeader[0].join("</th><th>");
    html += "</th></tr>";
    for (int i=1; i<tableWithHeader.size(); ++i) {
        html += QString("<tr style=\"background:%1\"><td" + cellStyle + ">").arg(
                    tableLineColors[i-1].color().name(QColor::HexRgb));
        html += tableWithHeader[i].join("</td><td" + cellStyle + ">");
        html += "</td></tr>";
    }
    html += "</table>";
    return html;
}
//----------------------------------------------------------
QList<PaneVatUE::ColInfo> PaneVatUE::colInfos() const
{
    static QList<PaneVatUE::ColInfo> colInfos
            = {{tr("Date"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getDateTime().toString(tr("dd/MM/yyyy hh:mm"));
                }}
               ,{tr("Numéro de commande"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->orderId();
                 }}
               ,{tr("ID Expédition / Remboursement"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getId();
                 }}
               ,{tr("Type"), [](const Shipment *shipmentOrRefund) -> QString{
                     if (dynamic_cast<const Refund *>(shipmentOrRefund) == nullptr) {
                         return QObject::tr("Vente");
                     }
                     return QObject::tr("Remboursement");
                 }}
               ,{tr("Facture"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getInvoiceNameMarketPlace();
                 }}
               ,{tr("Sous-channel"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->subchannel();
                 }}
               ,{tr("Expédié Vendeur"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getOrder()->getShippedBySeller()? tr("Oui"):tr("Non");
                 }}
               ,{tr("Montant TTC"), [](const Shipment *shipmentOrRefund) -> QString{
                    return QString::number(shipmentOrRefund->getTotalPriceTaxed(), 'f', 2);
                 }}
               ,{tr("Montant HT"), [](const Shipment *shipmentOrRefund) -> QString{
                    return QString::number(shipmentOrRefund->getTotalPriceUntaxed(), 'f', 2);
                 }}
               ,{tr("TVA"), [](const Shipment *shipmentOrRefund) -> QString{
                    return QString::number(shipmentOrRefund->getTotalPriceTaxes(), 'f', 2);
                 }}
               ,{tr("Professionnel"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getOrder()->isBusinessCustomer()? tr("Oui"):tr("Non");
                 }}
               ,{tr("Numéro de TVA"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getOrder()->getVatNumber();
                 }}
               ,{tr("Pays expédition"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getAddressFrom().countryName();
                 }}
               ,{tr("Pays destination"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getOrder()->getAddressTo().countryName();
                 }}
               ,{tr("Pays TVA"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getCountryCodeVat();
                 }}
               ,{tr("Régime TVA"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getRegimeVat();
                 }}
               ,{tr("Date commande"), [](const Shipment *shipmentOrRefund) -> QString{
                    return shipmentOrRefund->getOrder()->getDateTime().toString(tr("dd/MM/yyyy"));
                 }}
              };
    return colInfos;
}
//----------------------------------------------------------
