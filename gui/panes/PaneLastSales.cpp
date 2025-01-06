#include <QInputDialog>
#include <QFileDialog>
#include <QSettings>

#include "gui/panes/itemdelegates/SaleGroupsDelegate.h"

#include "model/inventory/SaleGroups.h"
#include "model/inventory/SalesLatestTable.h"

#include "PaneLastSales.h"
#include "ui_PaneLastSales.h"

const QString PaneLastSales::SETTING_DIR_GSPR_FILES{"PaneLastSales__GSPRdir"};

PaneLastSales::PaneLastSales(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneLastSales)
{
    ui->setupUi(this);
    ui->tableViewGroups->setModel(SaleGroups::instance());
    //ui->tableViewGroups->setItemDelegate(
                //new SaleGroupsDelegate{ui->tableViewGroups});
    m_updatingTextEditGroup = false;
    QSettings settings;
    ui->lineEditGsprFolder->setText(
                settings.value(
                    SETTING_DIR_GSPR_FILES, QString{}).toString());

    ui->dateEditFrom->setDate(QDate::currentDate().addMonths(-1));
    ui->dateEditTo->setDate(QDate::currentDate());
    _connectSlots();
}

void PaneLastSales::_connectSlots()
{
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
    connect(ui->buttonBrowseGsprFolder,
            &QPushButton::clicked,
            this,
            &PaneLastSales::browseGsprFolder);
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
    }
    setCursor(Qt::ArrowCursor);
}

void PaneLastSales::exportCsv()
{
    auto model = ui->tableSales->model();
    if (model != nullptr)
    {
        SalesLatestTable *salesModel = static_cast<SalesLatestTable *>(model);
        QSettings settings;
        QString key = "PaneLastSales__exportCsv";
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
            salesModel->exportCsv(filePath);
        }
    }
}

void PaneLastSales::browseGsprFolder()
{
    QSettings settings;
    QString lastDirPath = settings.value(
                SETTING_DIR_GSPR_FILES, QDir().absolutePath()).toString();
    QString directory = QFileDialog::getExistingDirectory(
                this, tr("Choisir un répertoire"),
                lastDirPath);
    if (!directory.isEmpty())
    {
        settings.setValue(SETTING_DIR_GSPR_FILES, directory);
    }
    ui->lineEditGsprFolder->setText(directory);
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


