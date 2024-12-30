#include <qfiledialog.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <QSettings>
#include <qmessagebox.h>
#include <qtextstream.h>
#include <QtCore/qdatetime.h>

#include "PaneInventory.h"
#include "ui_PaneInventory.h"

#include "../common/utils/CsvHeader.h"

#include "model/CustomerManager.h"
#include "model/SettingManager.h"
#include "model/orderimporters/Shipment.h"
#include "model/orderimporters/VatOrdersModel.h"
#include "model/orderimporters/ImporterYearsManager.h"
#include "model/inventory/InventoryManager.h"
#include "model/inventory/ManagerBundle.h"
#include "model/inventory/ManagerInventoryIssues.h"
#include "dialogs/DialogAddFileDate.h"

//----------------------------------------------------------
PaneInventory::PaneInventory(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneInventory)
{
    ui->setupUi(this);
    m_dialogAddInventory = nullptr;
    m_dialogAddPurchase = nullptr;
    m_dialogAddReturns = nullptr;
    ui->comboBoxYear->setModel(ImporterYearsManager::instance());
    ui->tableBundle->setModel(ManagerBundle::instance());
    ui->tableErrors->setModel(ManagerInventoryIssues::instance());
    ui->tableInventory->setModel(InventoryManager::instance());
    m_fileSystemModel = new QFileSystemModel(this);
    m_fileSystemModel->setReadOnly(true);
    QDir dirInventory = SettingManager::instance()->dirInventory();
    //m_fileSystemModel->setRootPath(dirInventory.absolutePath());
    ui->treeViewFilesImported->setModel(m_fileSystemModel);
    ui->treeViewFilesImported->header()->resizeSection(0, 250);
    //ui->treeViewFilesImported->setRootIndex(
                //m_fileSystemModel->index(dirInventory.absolutePath()));
    //ui->treeViewFilesImported->header()->resizeSection(0, 280);
    QString selectedCustomerId = CustomerManager::instance()->getSelectedCustomerId();
    ui->treeViewFilesImported->expandAll();
    if (!selectedCustomerId.isEmpty()) {
        onCustomerSelectedChanged(selectedCustomerId);
    }
    //ui->splitter->setSizes(QList<int>() << 50 << 100);
    _connectSlots();
}
//----------------------------------------------------------
PaneInventory::~PaneInventory()
{
    delete ui;
}
//----------------------------------------------------------
void PaneInventory::onCustomerSelectedChanged(const QString &customerId)
{
    if (customerId.isEmpty()) {
        // TODO reset
    } else {
        QDir dirInventory = SettingManager::instance()->dirInventory();
        m_fileSystemModel->setRootPath(dirInventory.absolutePath());
        ui->treeViewFilesImported->setRootIndex(
                    m_fileSystemModel->index(
                        dirInventory.absolutePath()));
        QSettings settings(SettingManager::instance()->settingsFilePath(),
                           QSettings::IniFormat);
        QString skusToExclude = settings.value(
                    _settingKeySkusExclude(), "").toString();
        ui->textEditExcludeCodes->setPlainText(skusToExclude);
    }
}
//----------------------------------------------------------
void PaneInventory::_connectSlots()
{
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            this,
            &PaneInventory::onCustomerSelectedChanged);
    connect(ui->buttonAddPurchase,
            &QPushButton::clicked,
            this,
            &PaneInventory::addPurchase);
    connect(ui->buttonAddInventoryBegin,
            &QPushButton::clicked,
            this,
            &PaneInventory::addInventoryFile);
    connect(ui->buttonAddReturn,
            &QPushButton::clicked,
            this,
            &PaneInventory::addReturns);
    connect(ui->buttonAddCodesEquivalent,
            &QPushButton::clicked,
            this,
            &PaneInventory::addCodesEquivalent);
    connect(ui->buttonAddBundles,
            &QPushButton::clicked,
            this,
            &PaneInventory::addBundles);
    connect(ui->buttonRemove,
            &QPushButton::clicked,
            this,
            &PaneInventory::removeSelectedFile);

    connect(ui->buttonExportUnknownCodes,
            &QPushButton::clicked,
            this,
            &PaneInventory::exportUnkknownCodes);
    connect(ui->buttonExportAllErrorCodes,
            &QPushButton::clicked,
            this,
            &PaneInventory::exportErrorCodes);

    connect(ui->buttonLoadInventory,
            &QPushButton::clicked,
            this,
            &PaneInventory::loadInventory);
    connect(ui->buttonSaveInventory,
            &QPushButton::clicked,
            this,
            &PaneInventory::saveInventory);
    connect(ui->buttonSaveUnsold,
            &QPushButton::clicked,
            this,
            &PaneInventory::saveInventoryNotSoldForOneYear);
    connect(ui->buttonFilter,
            &QPushButton::clicked,
            this,
            &PaneInventory::filter);
    connect(ui->buttonResetFilter,
            &QPushButton::clicked,
            this,
            &PaneInventory::clearFilter);
    connect(ui->buttonExportFiltered,
            &QPushButton::clicked,
            this,
            &PaneInventory::exportFilteredInventory);
    connect(ui->textEditExcludeCodes,
            &QPlainTextEdit::textChanged,
            [this]() {
        QString text = ui->textEditExcludeCodes->toPlainText();
        static int nChanging = 0;
        int *p_nChunging = &nChanging;
        ++nChanging;
        //static QDateTime lasteTimeChanged = QDateTime::currentDateTime().addDays(-1);
        //QDateTime p_lasteTimeChanged = &lasteTimeChanged;
        QTimer::singleShot(500, [this, p_nChunging](){
            --(*p_nChunging);
            if (*p_nChunging == 0) { /// We do this to avoid saving too much if user is typing
                QString text = ui->textEditExcludeCodes->toPlainText();
                QSettings settings(SettingManager::instance()->settingsFilePath(),
                                   QSettings::IniFormat);
                settings.setValue(this->_settingKeySkusExclude(), text);
            }
        });

    });
}
//----------------------------------------------------------
QString PaneInventory::_settingKeySkusExclude() const
{
    QString selectedCustomerId = CustomerManager::instance()->getSelectedCustomerId();
    return "PaneInventory_settingKeySkusExclude_" + selectedCustomerId;
}
//----------------------------------------------------------
QSet<QString> PaneInventory::_codesToExclude() const
{
    QStringList codes = ui->textEditExcludeCodes->toPlainText().split("\n");
    QSet<QString> codesSet;
    for (int i=codes.size()-1; i>=0; --i) {
        if (codes[i].trimmed().isEmpty()) {
            //codes.removeAt(i);
        } else {
            QString mainCode = InventoryManager::instance()->mainCode(codes[i]);
            codesSet << mainCode;
        }
    }
    return codesSet;
}
//----------------------------------------------------------
void PaneInventory::addPurchase()
{
    if (m_dialogAddPurchase == nullptr) {
        m_dialogAddPurchase = new DialogAddFileDate(
                    "PaneInventory-purchase", false, true, this);
    }
    connect(m_dialogAddPurchase,
            &DialogAddFileDate::accepted,
            [this](){
        QStringList filePaths = m_dialogAddPurchase->getFilePaths();
        QList<QDate> dates = m_dialogAddPurchase->getDates();
        for (int i=0; i<filePaths.size(); ++i) {
            try {
                InventoryManager::instance()->addPurchaseFile(
                            filePaths[i], dates[i]);
            }  catch (const CsvHeaderException &exception) {
                QMessageBox::critical(
                            this, tr("Erreur entête ") + QFileInfo(filePaths[i]).fileName(),
                            tr("L'entête du fichier ne contient pas l'une de ces colonnes ")
                            + exception.columnValuesError().join(" ,"),
                            QMessageBox::Ok);
                break;
            }
        }
    });
    m_dialogAddPurchase->show();
}
//----------------------------------------------------------
void PaneInventory::addInventoryFile()
{
    if (m_dialogAddInventory == nullptr) {
        m_dialogAddInventory = new DialogAddFileDate(
                    "PaneInventory-inventory-year", true, false, this);
    }
    connect(m_dialogAddInventory,
            &DialogAddFileDate::accepted,
            [this](){
        QString filePath = m_dialogAddInventory->getFilePaths()[0];
        int year = m_dialogAddInventory->getYears()[0];
        try {
            InventoryManager::instance()->addInventoryBeginFile(
                        filePath, year);
        }  catch (const CsvHeaderException &exception) {
            QMessageBox::critical(
                        this, tr("Erreur dans l'entête"),
                        tr("L'entête du fichier ne contient pas l'une de ces colonnes ")
                        + exception.columnValuesError().join(" ,"),
                        QMessageBox::Ok);
        }
    });
    m_dialogAddInventory->show();
}
//----------------------------------------------------------
void PaneInventory::addReturns()
{
    if (m_dialogAddReturns == nullptr) {
        m_dialogAddReturns = new DialogAddFileDate(
                    "PaneInventory-returns", false, true, this);
    }
    connect(m_dialogAddReturns,
            &DialogAddFileDate::accepted,
            [this](){
        QStringList filePaths = m_dialogAddReturns->getFilePaths();
        QList<QDate> dates = m_dialogAddReturns->getDates();
        for (int i=0; i<filePaths.size(); ++i) {
            InventoryManager::instance()->addAmazonReturnFile(
                        filePaths[i], dates[i]);
        }
    });
    m_dialogAddReturns->show();
}
//----------------------------------------------------------
void PaneInventory::addCodesEquivalent()
{
    QSettings settings;
    QString key = "PaneInventory__addCodesEquivalent" + CustomerManager::instance()->getSelectedCustomerId();
    QString lastDirPath = settings.value(
                key, QDir().absolutePath()).toString();
    QString filePath = QFileDialog::getOpenFileName(
                this, tr("Choisir un fichier"),
                lastDirPath,
                "CSV (*.csv)");
    if (!filePath.isEmpty()) {
        QFileInfo fileInfo(filePath);
        settings.setValue(key, fileInfo.absoluteDir().absolutePath());
        InventoryManager::instance()->addMergingCodeFile(
                    filePath);
    }
}
//----------------------------------------------------------
void PaneInventory::addBundles()
{
    QSettings settings;
    QString key = "PaneInventory__addBundles" + CustomerManager::instance()->getSelectedCustomerId();
    QString lastDirPath = settings.value(
                key, QDir().absolutePath()).toString();
    QString filePath = QFileDialog::getOpenFileName(
                this, tr("Choisir un fichier"),
                lastDirPath,
                "CSV (*.csv)");
    if (!filePath.isEmpty()) {
        QFileInfo fileInfo(filePath);
        settings.setValue(key, fileInfo.absoluteDir().absolutePath());
        ManagerBundle::instance()->addBundleFile(filePath);
    }
}
//----------------------------------------------------------
void PaneInventory::removeSelectedFile()
{
    auto selIndexes
            = ui->treeViewFilesImported->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0) {
        QString fileName = selIndexes.first().data().toString();
        QModelIndex parentIndex = selIndexes.first().parent();
        bool yearFound = false;
        QString inventoryRelDir;
        do {
            QString dirName = parentIndex.data().toString();
            dirName.toInt(&yearFound);
            inventoryRelDir.insert(0, dirName + QDir::separator());
            parentIndex = parentIndex.parent();
        } while (!yearFound && parentIndex.isValid());
        QDir dirFilePath
                = SettingManager::instance()->dirInventory().filePath(
                    inventoryRelDir);
        QString filePathToRemove
                = dirFilePath.filePath(fileName);
        if (QFile::exists(filePathToRemove)) {
            QFile::remove(filePathToRemove);
        }
    }
}
//----------------------------------------------------------
void PaneInventory::exportUnkknownCodes()
{
    QSettings settings;
    QString key = "PaneInventory__exportUnkknownCodes" + CustomerManager::instance()->getSelectedCustomerId();
    QString lastDirPath = settings.value(
                key, QDir().absolutePath()).toString();
    QString filePath = QFileDialog::getSaveFileName(
                this, tr("Choisir un fichier"),
                lastDirPath,
                "CSV (*.csv)");
    if (!filePath.isEmpty()) {
        QFileInfo fileInfo(filePath);
        settings.setValue(key, fileInfo.absoluteDir().absolutePath());
        ManagerInventoryIssues::instance()->exportUnknown(filePath);
    }
}
//----------------------------------------------------------
void PaneInventory::exportErrorCodes()
{
    QSettings settings;
    QString key = "PaneInventory__exportUnkknownCodes" + CustomerManager::instance()->getSelectedCustomerId();
    QString lastDirPath = settings.value(
                key, QDir().absolutePath()).toString();
    QString filePath = QFileDialog::getSaveFileName(
                this, tr("Choisir un fichier"),
                lastDirPath,
                "CSV (*.csv)");
    if (!filePath.isEmpty()) {
        QFileInfo fileInfo(filePath);
        settings.setValue(key, fileInfo.absoluteDir().absolutePath());
        ManagerInventoryIssues::instance()->exportAll(filePath);
    }
}
//----------------------------------------------------------
void PaneInventory::exportFilteredInventory()
{
    bool hasYear = false;
    ui->comboBoxYear->currentText().toInt(&hasYear);
    if (hasYear) {
        QSettings settings;
        QString key = "PaneInventory__exportFilteredInventory" + CustomerManager::instance()->getSelectedCustomerId();
        QString lastDirPath = settings.value(
                    key, QDir().absolutePath()).toString();
        QString filePath = QFileDialog::getSaveFileName(
                    this, tr("Choisir un fichier"),
                    lastDirPath,
                    "CSV (*.csv)");
        if (!filePath.isEmpty()) {
            QFileInfo fileInfo(filePath);
            settings.setValue(key, fileInfo.absoluteDir().absolutePath());
            QStringList lines;
            auto model = InventoryManager::instance();
            int nCols = model->columnCount();
            int nRows = model->rowCount();
            QStringList elementsHeader;
            for (int i=0; i<nCols; ++i) {
                elementsHeader << model->headerData(i, Qt::Horizontal).toString();
            }
            QString sep = "\t";
            lines << elementsHeader.join(sep);
            for (int i=0; i<nRows; ++i) {
                if (!ui->tableInventory->isRowHidden(i)) {
                    QStringList elements;
                    for (int j=0; j<nCols; ++j) {
                        elements << model->data(
                                        model->index(i, j)).toString();
                    }
                    lines << elements.join(sep);
                }
            }
            QFile file(filePath);
            if (file.open(QFile::WriteOnly)) {
                QTextStream stream(&file);
                stream << lines.join("\n");
                file.close();
            }
        }
    }
}
//----------------------------------------------------------
void PaneInventory::loadInventory()
{
    bool hasYear = false;
    int year = ui->comboBoxYear->currentText().toInt(&hasYear);
    if (hasYear) {
        setCursor(Qt::WaitCursor);
        ManagerInventoryIssues::instance()->clear();
        InventoryManager::instance()->load(year);
        VatOrdersModel::instance()->computeVat(
                    year,
                    [](const Shipment *shipment){
            if (!shipment->isRefund()) {
                QDate date = shipment->getDateTime().date();
                for (auto article : shipment->getArticlesShipped()) {
                    InventoryManager::instance()->recordMovement(
                                article->getSku(),
                                article->getName(),
                                article->getUnits(),
                                date);
                }
            }
        });
        InventoryManager::instance()->refresh(year);
        double inventoryValue = InventoryManager::instance()->inventoryValue();
        QString text = QString::number(inventoryValue, 'f', 2)
                + " " + CustomerManager::instance()->getSelectedCustomerCurrency();
        ui->labelInventoryValue->setText(text);
        setCursor(Qt::ArrowCursor);
    }
}
//----------------------------------------------------------
void PaneInventory::saveInventory()
{
    bool hasYear = false;
    ui->comboBoxYear->currentText().toInt(&hasYear);
    if (hasYear) {
        QSettings settings;
        QString key = "PaneInventory__saveInventory" + CustomerManager::instance()->getSelectedCustomerId();
        QString lastDirPath = settings.value(
                    key, QDir().absolutePath()).toString();
        QString filePath = QFileDialog::getSaveFileName(
                    this, tr("Choisir un fichier"),
                    lastDirPath,
                    "CSV (*.csv)");
        if (!filePath.isEmpty()) {
            QFileInfo fileInfo(filePath);
            settings.setValue(key, fileInfo.absoluteDir().absolutePath());
            InventoryManager::instance()->exportInventory(filePath);
        }
    }
}
//----------------------------------------------------------
void PaneInventory::saveInventoryNotSoldForOneYear()
{
    bool hasYear = false;
    ui->comboBoxYear->currentText().toInt(&hasYear);
    if (hasYear) {
        QSettings settings;
        QString key = "PaneInventory__saveInventory" + CustomerManager::instance()->getSelectedCustomerId();
        QString lastDirPath = settings.value(
                    key, QDir().absolutePath()).toString();
        QString filePath = QFileDialog::getSaveFileName(
                    this, tr("Choisir un fichier"),
                    lastDirPath,
                    "CSV (*.csv)");
        if (!filePath.isEmpty()) {
            QFileInfo fileInfo(filePath);
            settings.setValue(key, fileInfo.absoluteDir().absolutePath());
            InventoryManager::instance()->exportInventoryUnsold(filePath);
        }
    }
}
//----------------------------------------------------------
void PaneInventory::filter()
{
    QString codeFilter = ui->lineEditCode->text();
    QString nameFilter = ui->lineEditName->text();
    QString codeParentFilter = ui->lineEditCodeParent->text();
    auto codesToExclude = _codesToExclude();

    bool searchCode = !codeFilter.isEmpty();
    bool searchName = !nameFilter.isEmpty();
    bool searchCodeParent = !codeParentFilter.isEmpty();

    bool codeContains = ui->checkBoxCodeContains->isChecked();
    //bool nameContains = ui->checkBoxProductNameContains->isChecked();
    bool codeParentContains = ui->checkBoxCodeParentContains->isChecked();

    bool checkInventory = ui->checkBoxMonthInventory->isChecked();
    int leftMonthsFilter = ui->spinBoxMonthsLeft->value();
    bool checkAnnualSales = ui->checkBoxMonthInventory->isChecked();
    int annualSalesMinFilter = ui->spinBoxAnnualSalesMin->value();

    int indCode = InventoryManager::instance()->indexColumn(
                InventoryManager::COL_CODE);
    int indCodeParent = InventoryManager::instance()->indexColumn(
                InventoryManager::COL_CODE_PARENT);
    int indTitles = InventoryManager::instance()->indexColumn(
                InventoryManager::COL_TITLES);
    int indLeftMonth = InventoryManager::instance()->indexColumn(
                InventoryManager::COL_LEFT_MONTHS);
    int indSales365j = InventoryManager::instance()->indexColumn(
                InventoryManager::COL_SALES_365J);
    auto model = InventoryManager::instance();
    int nRows = model->rowCount();
    bool display = true;
    for (int i=0; i<nRows; ++i) {
        if (searchCode) {
            QString valueCode = model->data(
                        model->index(i, indCode)).toString();
            if (!codesToExclude.contains(valueCode)) {
                if (codeContains) {
                    display = valueCode.contains(codeFilter);
                } else {
                    display = valueCode == codeFilter;
                }
            } else {
                display = false;
            }
        }
        if (display && searchName) {
            QString valueTitles = model->data(
                 model->index(i, indTitles)).toString();
            display = valueTitles.contains(nameFilter);
        }
        if (display && searchCodeParent) {
            QString valueCodeParent = model->data(
                 model->index(i, indCodeParent)).toString();
            if (codeParentContains) {
                display = valueCodeParent.contains(codeParentFilter);
            } else {
                display = valueCodeParent == codeParentFilter;
            }
        }
        if (display && checkInventory) {
            double valueLeftMonths = model->data(
                 model->index(i, indLeftMonth)).toDouble();
            display = valueLeftMonths <= leftMonthsFilter;
        }
        if (display && checkAnnualSales) {
            int valueSales365j = model->data(
                 model->index(i, indSales365j)).toInt();
            display = valueSales365j >= annualSalesMinFilter;
        }
        ui->tableInventory->setRowHidden(i, !display);
    }
}
//----------------------------------------------------------
void PaneInventory::clearFilter()
{
    ui->lineEditCode->clear();
    ui->lineEditName->clear();
    ui->lineEditCodeParent->clear();
    ui->checkBoxCodeContains->setChecked(true);
    ui->checkBoxCodeParentContains->setChecked(true);
    ui->checkBoxProductNameContains->setChecked(true);
    ui->spinBoxMonthsLeft->setValue(2);
    int nRows = ui->tableInventory->model()->rowCount();
    for (int i=0; i<nRows; ++i) {
        ui->tableInventory->setRowHidden(0, false);
    }
}
//----------------------------------------------------------

