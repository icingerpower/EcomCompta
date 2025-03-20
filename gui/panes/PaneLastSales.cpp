#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>

#include "../common/utils/CsvHeader.h"

#include "gui/panes/dialogs/DialogEditSalesTemplate.h"

#include "model/SettingManager.h"
#include "model/inventory/SaleGroups.h"
#include "model/inventory/SaleTemplateManager.h"
#include "model/inventory/SalesLatestTable.h"
#include "model/inventory/CsvOrderFolders.h"
#include "model/inventory/SaleColumnTree.h"

#include "PaneLastSales.h"
#include "ui_PaneLastSales.h"


const QString PaneLastSales::SETTING_DIR_ECONOMICS{"PaneLastSales_dir_economics"};

PaneLastSales::PaneLastSales(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneLastSales)
{
    ui->setupUi(this);
    ui->tableViewGroups->setModel(SaleGroups::instance());
    ui->listViewCsvOrderFolders->setModel(CsvOrderFolders::instance());
    m_updatingTextEditGroup = false;

    ui->dateEditFrom->setDate(QDate::currentDate().addMonths(-1));
    ui->dateEditTo->setDate(QDate::currentDate());
    ui->comboBoxTemplates->setModel(SaleTemplateManager::instance());

        QSettings settingsCommon(SettingManager::instance()->settingsFilePath(),
                           QSettings::IniFormat);
    ui->lineEditEconomicsFolder->setText(
            settingsCommon.value(SETTING_DIR_ECONOMICS).toString());
    _connectSlots();
}

void PaneLastSales::_connectSlots()
{
    connect(ui->buttonEditTemplates,
            &QPushButton::clicked,
            this,
            &PaneLastSales::editTemplates);
    connect(ui->buttonAddGroup,
            &QPushButton::clicked,
            this,
            &PaneLastSales::addGroup);
    connect(ui->buttonRemoveGroup,
            &QPushButton::clicked,
            this,
            &PaneLastSales::removeGroupSelected);
    connect(ui->textEditKeywordsSkus,
            &QTextEdit::textChanged,
            this,
            &PaneLastSales::onGroupKeywordsEdited);
    connect(ui->tableViewGroups->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &PaneLastSales::onGroupSelected);
    connect(ui->buttonBrowseEconomicsFolder,
            &QPushButton::clicked,
            this,
            &PaneLastSales::browseEconomicsFolder);
    connect(ui->buttonAddFolder,
            &QPushButton::clicked,
            this,
            &PaneLastSales::browseGsprFolder);
    connect(ui->buttonRemoveFolder,
            &QPushButton::clicked,
            this,
            &PaneLastSales::removeFolder);
    connect(ui->buttonCompute,
            &QPushButton::clicked,
            this,
            &PaneLastSales::compute);
    connect(ui->buttonExport,
            &QPushButton::clicked,
            this,
            &PaneLastSales::exportCsv);
}

PaneLastSales::~PaneLastSales()
{
    delete ui;
}

void PaneLastSales::editTemplates()
{
    DialogEditSalesTemplate dialog;
    dialog.exec();
}

void PaneLastSales::addGroup()
{
    const QString &name = QInputDialog::getText(
                this,
                tr("Nom"),
                tr("Entrez le nom du groupe"));
    if (!name.isEmpty())
    {
        SaleGroups::instance()->add(name);
    }
}

void PaneLastSales::removeGroupSelected()
{
    auto selItems = ui->tableViewGroups->selectionModel()->selectedIndexes();
    if (selItems.size() > 0)
    {
        SaleGroups::instance()->remove(selItems.first());
    }
}

void PaneLastSales::compute()
{
    setCursor(Qt::WaitCursor);
    auto selGroups = ui->tableViewGroups->selectionModel()->selectedIndexes();
    if (selGroups.size() > 0)
    {
        SalesLatestTable *salesModel = nullptr;
        auto model = ui->tableSales->model();
        if (model == nullptr)
        {
            salesModel = new SalesLatestTable{ui->tableSales};
        }
        else
        {
            salesModel = static_cast<SalesLatestTable *>(model);
        }
        auto indexGroup = selGroups.first();
        const QSet<QString> &keywordSkus = SaleGroups::instance()->getKeywordsSkusAsSet(indexGroup);
        const auto &amazons = SaleGroups::instance()->getAmazons(indexGroup);
        salesModel->compute(
                    keywordSkus,
                    amazons,
                    ui->dateEditFrom->date(),
                    ui->dateEditTo->date());
        ui->tableSales->setModel(salesModel);
    }
    else
    {
        QMessageBox::information(this,
                                 tr("Pas de groupe sélectionné"),
                                 tr("Vous devez sélectionner un groupe."));
    }
    setCursor(Qt::ArrowCursor);
}

void PaneLastSales::exportCsv()
{
    auto model = ui->tableSales->model();
    if (model != nullptr)
    {
        auto selGroups = ui->tableViewGroups->selectionModel()->selectedIndexes();
        if (selGroups.size() > 0)
        {
            auto indexGroup = selGroups.first();
            const auto &extAmazons = SaleGroups::instance()->getExtAmazons(indexGroup);
            SalesLatestTable *salesModel = static_cast<SalesLatestTable *>(model);
            QSettings settings;
            QString key = "PaneLastSales__exportCsv";
            QString lastDirPath = settings.value(
                                              key, QDir().absolutePath()).toString();
            QString filePath = QFileDialog::getSaveFileName(
                this, tr("Choisir un fichier"),
                lastDirPath,
                "CSV (*.csv)");
            if (!filePath.isEmpty())
            {
                if (!filePath.toLower().endsWith(".csv"))
                {
                    filePath += ".csv";
                }
                int templateIndex = ui->comboBoxTemplates->currentIndex();
                const auto &templateId = SaleTemplateManager::instance()->getId(templateIndex);
                settings.setValue(key, QFileInfo{filePath}.dir().path());
                const QString &idTree
                    = SaleColumnTree::createId(templateId);
                SaleColumnTree saleColumnTree{idTree};
                const auto &minDate = ui->dateEditFrom->date();
                const auto &dirEconomics = ui->lineEditEconomicsFolder->text();
                try
                {
                    salesModel->exportCsv(filePath, &saleColumnTree, dirEconomics, extAmazons, minDate);
                } catch (const CsvHeaderException &exception)
                {
                    QMessageBox::information(
                        this,
                        tr("Erreur format CSV"),
                        tr("Colonnes manquantes") + " - " + QFileInfo{filePath}.fileName() + ": " + exception.columnValuesError().join(" - "));
                }
            }
        }
        else
        {
            QMessageBox::information(this,
                                     tr("Pas de groupe sélectionné"),
                                     tr("Vous devez sélectionner un groupe."));
        }
    }
}

void PaneLastSales::browseGsprFolder()
{
    QSettings settings;
    QString settingKey{"PaneLastSales__GSPRdir"};
    QString lastDirPath = settings.value(
                settingKey, QDir().absolutePath()).toString();
    const QString &folder = QFileDialog::getExistingDirectory(
                this, tr("Choisir un répertoire"),
                lastDirPath);
    if (!folder.isEmpty())
    {
        settings.setValue(settingKey, folder);
        CsvOrderFolders::instance()->add(folder);
    }
}

void PaneLastSales::browseEconomicsFolder()
{
    QSettings settings;
    QString settingKey{"PaneLastSales__economicsFolder"};
    QString lastDirPath = settings.value(
                settingKey, QDir().absolutePath()).toString();
    const QString &folder = QFileDialog::getExistingDirectory(
                this, tr("Choisir un répertoire"),
                lastDirPath);
    if (!folder.isEmpty())
    {
        settings.setValue(settingKey, folder);
        ui->lineEditEconomicsFolder->setText(folder);
        QSettings settingsCommon(SettingManager::instance()->settingsFilePath(),
                           QSettings::IniFormat);
        settingsCommon.setValue(SETTING_DIR_ECONOMICS, folder);
    }
}

void PaneLastSales::removeFolder()
{
    const auto &selIndexes
        = ui->listViewCsvOrderFolders->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        CsvOrderFolders::instance()->remove(selIndexes.first());
    }
}

void PaneLastSales::onGroupKeywordsEdited()
{
    auto selItems = ui->tableViewGroups->selectionModel()->selectedIndexes();
    if (selItems.size() > 0 && !m_updatingTextEditGroup)
    {
        SaleGroups::instance()->setKeywordsSkus(
                    selItems.first(),
                    ui->textEditKeywordsSkus->toPlainText());
    }
}

void PaneLastSales::onGroupSelected(
        const QItemSelection &newSelection,
        const QItemSelection &previousSelection)
{
    m_updatingTextEditGroup = true;
    if (newSelection.size() > 0)
    {
        auto index = newSelection.indexes().first();
        ui->textEditKeywordsSkus->setPlainText(
                    SaleGroups::instance()->getKeywordsSkus(index).join("\n"));
    }
    else if (previousSelection.size() > 0)
    {
        ui->textEditKeywordsSkus->clear();
    }
    m_updatingTextEditGroup = false;
}


