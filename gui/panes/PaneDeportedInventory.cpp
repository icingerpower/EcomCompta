#include "PaneDeportedInventory.h"
#include "ui_PaneDeportedInventory.h"

#include "model/orderimporters/ModelStockDeported.h"
#include "model/CustomerManager.h"

//----------------------------------------------------------
PaneDeportedInventory::PaneDeportedInventory(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneDeportedInventory)
{
    ui->setupUi(this);
    ui->tableViewPrices->setModel(ModelStockDeported::instance());
    ui->tableViewPrices->horizontalHeader()->resizeSection(0, 180);
    ui->tableViewPrices->horizontalHeader()->resizeSection(1, 300);
    _connectSlots();
    QString selectedCustomerId = CustomerManager::instance()->getSelectedCustomerId();
    if (!selectedCustomerId.isEmpty()) {
        onCustomerSelectedChanged(selectedCustomerId);
    }
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            this,
            &PaneDeportedInventory::onCustomerSelectedChanged);
}
//----------------------------------------------------------
void PaneDeportedInventory::onCustomerSelectedChanged(
        const QString &)
{
    _disconnectSlots();
    auto computingType = ModelStockDeported::instance()->computingType();
    if (computingType == StockDeportedComputing::DontCompute) {
        ui->radioNoCompute->setChecked(true);
        ui->stackedWidget->setCurrentIndex(0);
    } else if (computingType == StockDeportedComputing::TableValues) {
        ui->radioExactValues->setChecked(true);
        ui->stackedWidget->setCurrentIndex(1);
    } else if (computingType == StockDeportedComputing::Percentage) {
        ui->radioPercentage->setChecked(true);
        ui->stackedWidget->setCurrentIndex(2);
    }
    ui->spinBoxPercentage->setValue(
                ModelStockDeported::instance()->percentage());
    _connectSlots();
}
//----------------------------------------------------------
PaneDeportedInventory::~PaneDeportedInventory()
{
    delete ui;
}
//----------------------------------------------------------
void PaneDeportedInventory::resetSelection()
{
    auto selection = ui->tableViewPrices->selectionModel()->selectedRows();
    while(selection.size() > 0) {
        auto sel = selection.takeLast();
        ModelStockDeported::instance()->resetValue(sel);
    }
}
//----------------------------------------------------------
void PaneDeportedInventory::_connectSlots()
{
    connect(ui->buttonReset,
            &QPushButton::clicked,
            this,
            &PaneDeportedInventory::resetSelection);
    connect(ui->buttonComputeFromInventoryFiles,
            &QPushButton::clicked,
            this,
            &PaneDeportedInventory::computeFromInventoryFiles);
    connect(ui->radioNoCompute,
            &QRadioButton::clicked,
            [this](bool checked) {
        if (checked) {
            ui->stackedWidget->setCurrentIndex(0);
            ModelStockDeported::instance()->setComputingType(
                        StockDeportedComputing::DontCompute);
        }
    });
    connect(ui->spinBoxPercentage,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            ModelStockDeported::instance(),
            &ModelStockDeported::setPercentage);
    connect(ui->radioExactValues,
            &QRadioButton::clicked,
            [this](bool checked) {
        if (checked) {
            ui->stackedWidget->setCurrentIndex(1);
            ModelStockDeported::instance()->setComputingType(
                        StockDeportedComputing::TableValues);
        }
    });
    connect(ui->radioPercentage,
            &QRadioButton::clicked,
            [this](bool checked) {
        if (checked) {
            ui->stackedWidget->setCurrentIndex(2);
            ModelStockDeported::instance()->setComputingType(
                        StockDeportedComputing::Percentage);
        }
    });
}
//----------------------------------------------------------
void PaneDeportedInventory::computeFromInventoryFiles()
{
    ModelStockDeported::instance()->loadFromInventoryManager();
    ModelStockDeported::instance()->saveInSettings();
}
//----------------------------------------------------------
void PaneDeportedInventory::_disconnectSlots()
{
    disconnect(ui->buttonReset);
    disconnect(ui->radioNoCompute);
    disconnect(ui->spinBoxPercentage);
    disconnect(ui->radioExactValues);
    disconnect(ui->radioPercentage);
}
//----------------------------------------------------------
