#include <QInputDialog>


#include "model/inventory/SaleTemplateManager.h"
#include "model/inventory/SaleColumnTree.h"

#include "DialogEditSalesTemplate.h"
#include "ui_DialogEditSalesTemplate.h"

DialogEditSalesTemplate::DialogEditSalesTemplate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditSalesTemplate)
{
    ui->setupUi(this);
    ui->listViewTemplates->setModel(SaleTemplateManager::instance());
    ui->widgetMenuRight->setEnabled(false);
    _connectSlots();
}

void DialogEditSalesTemplate::_connectSlots()
{
    connect(ui->buttonAddTemplate,
            &QPushButton::clicked,
            this,
            &DialogEditSalesTemplate::addTemplate);
    connect(ui->buttonRemoveTemplate,
            &QPushButton::clicked,
            this,
            &DialogEditSalesTemplate::removeTemplateSelected);
    connect(ui->buttonAddTopColumn,
            &QPushButton::clicked,
            this,
            &DialogEditSalesTemplate::addColumnTop);
    connect(ui->buttonAddColumn,
            &QPushButton::clicked,
            this,
            &DialogEditSalesTemplate::addColumn);
    connect(ui->buttonRemoveColumn,
            &QPushButton::clicked,
            this,
            &DialogEditSalesTemplate::removeColumnSelected);
    connect(ui->buttonUp,
            &QPushButton::clicked,
            this,
            &DialogEditSalesTemplate::upColumn);
    connect(ui->buttonDown,
            &QPushButton::clicked,
            this,
            &DialogEditSalesTemplate::downColumn);
    connect(ui->buttonUnselect,
            &QPushButton::clicked,
            this,
            &DialogEditSalesTemplate::unselect);
    connect(ui->listViewTemplates->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &DialogEditSalesTemplate::onTemplateSelected);
}

DialogEditSalesTemplate::~DialogEditSalesTemplate()
{
    delete ui;
}

SaleColumnTree *DialogEditSalesTemplate::saleColmunTreeModel() const
{
    auto model = ui->treeViewColumns->model();
    if (model != nullptr)
    {
        return static_cast<SaleColumnTree *>(model);
    }
    return nullptr;
}

void DialogEditSalesTemplate::addTemplate()
{
    const QString &name = QInputDialog::getText(
                this,
                tr("Nom"),
                tr("Entrez le nom du template"));
    if (!name.isEmpty())
    {
        SaleTemplateManager::instance()->add(name);
        if (SaleTemplateManager::instance()->rowCount() == 1)
        {
            ui->listViewTemplates->setCurrentIndex(
                        SaleTemplateManager::instance()->index(0, 0));
        }
    }
}

void DialogEditSalesTemplate::removeTemplateSelected()
{
    const auto &indexes = ui->listViewTemplates->selectionModel()->selectedIndexes();
    if (indexes.size() > 0)
    {
        SaleTemplateManager::instance()->remove(indexes.first());
    }
}

void DialogEditSalesTemplate::addColumnTop()
{
    const QString &colName = QInputDialog::getText(
                this,
                tr("Nom"),
                tr("Entrez le nom de la colonne"));
    if (!colName.isEmpty())
    {
        saleColmunTreeModel()->addItem(QModelIndex{}, colName);
    }
}

void DialogEditSalesTemplate::addColumn()
{
    const QString &colName = QInputDialog::getText(
                this,
                tr("Nom"),
                tr("Entrez le nom de la colonne"));
    if (!colName.isEmpty())
    {
        const auto &selIndexes = ui->treeViewColumns->selectionModel()->selectedIndexes();
        QModelIndex parent;
        if (selIndexes.size() > 0)
        {
            parent = selIndexes.first();
        }
        saleColmunTreeModel()->addItem(parent, colName);
        if (parent.isValid())
        {
            ui->treeViewColumns->expand(parent);
        }
    }
}

void DialogEditSalesTemplate::removeColumnSelected()
{
    const auto &selIndexes = ui->treeViewColumns->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        saleColmunTreeModel()->removeItem(selIndexes.first());
    }
}

void DialogEditSalesTemplate::upColumn()
{
    const auto &selIndexes = ui->treeViewColumns->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        auto index = selIndexes.first();
        int newIndexRow = index.row()-1;
        if (newIndexRow > -1)
        {
            ui->treeViewColumns->selectionModel()->clearSelection();
            saleColmunTreeModel()->upItem(index);
            ui->treeViewColumns->setCurrentIndex(index.siblingAtRow(newIndexRow));
        }
    }
}

void DialogEditSalesTemplate::downColumn()
{
    const auto &selIndexes = ui->treeViewColumns->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        auto index = selIndexes.first();
        int newIndexRow = index.row()+1;
        if (newIndexRow < ui->treeViewColumns->model()->rowCount()-1)
        {
            ui->treeViewColumns->selectionModel()->clearSelection();
            saleColmunTreeModel()->downItem(index);
            ui->treeViewColumns->setCurrentIndex(index.siblingAtRow(newIndexRow));
        }
    }
}

void DialogEditSalesTemplate::unselect()
{
    ui->treeViewColumns->clearSelection();
}

void DialogEditSalesTemplate::onTemplateSelected(
        const QItemSelection &newSelection,
        const QItemSelection &oldSelection)
{
    ui->widgetMenuRight->setEnabled(false);
    if (newSelection.size() > 0)
    {
        const QString &idTemplate = SaleTemplateManager::instance()
                ->getId(newSelection.indexes().first());
        const QString &idTree
                = idTemplate + "_" + CustomerManager::instance()->getSelectedCustomerId();
        auto saleTreeColumn = new SaleColumnTree{idTree, ui->treeViewColumns};
        ui->treeViewColumns->setModel(saleTreeColumn);
        ui->widgetMenuRight->setEnabled(true);
    }
    else if (oldSelection.size() > 0)
    {
        auto model = ui->treeViewColumns->model();
        ui->treeViewColumns->setModel(nullptr);
        model->deleteLater();
    }
}

