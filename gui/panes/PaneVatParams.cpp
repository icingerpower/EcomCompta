#include <qmessagebox.h>
#include <qinputdialog.h>

#include "PaneVatParams.h"
#include "ui_PaneVatParams.h"

#include "model/vat/VatRatesModel.h"
#include "model/vat/SelectedSkusListModel.h"
#include "model/vat/VatRateManager.h"
#include "model/orderimporters/SkusFoundManager.h"
#include "model/orderimporters/VatOrdersModel.h"
#include "model/orderimporters/ImporterYearsManager.h"
#include "model/orderimporters/Shipment.h"
#include "dialogs/DialogOtherVatRates.h"

//----------------------------------------------------------
PaneVatParams::PaneVatParams(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneVatParams)
{
    ui->setupUi(this);
    ui->comboBoxYear->setModel(ImporterYearsManager::instance());
    //ui->tableDefaultVat->setModel(
                //VatRateManager::instance()->getDefautVatModel());
    ui->widgetVatRatesDefault->setModels(
                VatRateManager::instance()->getDefautVatModel(),
                VatRateManager::instance()->getDefautVatModelDates());
    ui->listOtherRates->setModel(VatRateManager::instance());
    ui->listKnownSkus->setModel(SkusFoundManager::instance());
    m_dialogOtherVatRates = nullptr;
    _hideOtherRatesFields();
    _connectSlots();
}
//----------------------------------------------------------
void PaneVatParams::_connectSlots()
{
    connect(ui->buttonAddSku,
            &QPushButton::clicked,
            this,
            &PaneVatParams::addSku);
    connect(ui->buttonAddRates,
            &QPushButton::clicked,
            this,
            &PaneVatParams::addOtherRates);
    connect(ui->buttonRemoveSku,
            &QPushButton::clicked,
            this,
            &PaneVatParams::removeSelectedSkus);
    connect(ui->buttonAddToRight,
            &QPushButton::clicked,
            this,
            &PaneVatParams::addSkusToRight);
    connect(ui->buttonRemoveRates,
            &QPushButton::clicked,
            this,
            &PaneVatParams::removeSelectedOtherRates);
    connect(ui->buttonRemoveFromRight,
            &QPushButton::clicked,
            this,
            &PaneVatParams::removeSkusFromRight);
    connect(ui->listOtherRates->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &PaneVatParams::displaySelOtherRates);
    connect(ui->lineEditFilterKnownSkus,
            &QLineEdit::textEdited,
            SkusFoundManager::instance(),
            &SkusFoundManager::filters);
    connect(ui->buttonLoadYearSkus,
            &QPushButton::clicked,
            this,
            &PaneVatParams::loadYearSkus);
}
//----------------------------------------------------------
void PaneVatParams::_hideOtherRatesFields()
{
    ui->widgetOtherRates->hide();
    ui->verticalLayoutMain->addItem(ui->horizontalSpacerMain);
}
//----------------------------------------------------------
void PaneVatParams::_showOtherRatesFields()
{
    ui->widgetOtherRates->show();
    ui->verticalLayoutMain->removeItem(ui->horizontalSpacerMain);
}
//----------------------------------------------------------
PaneVatParams::~PaneVatParams()
{
    _showOtherRatesFields();
    delete ui;
}
//----------------------------------------------------------
/*
void PaneVatParams::loadCustomer(const QString &customerId)
{
    VatRateManager::instance()->loadFromSettings();
    ui->tableOtherRates->setModel(
                VatRateManager::instance()->getOtherVatModel(name).data());
    ui->listSelectedSkus->setModel(
                VatRateManager::instance()->getSelectedSkusModel(name).data());
}
//*/
//----------------------------------------------------------
void PaneVatParams::addOtherRates()
{
    if (m_dialogOtherVatRates == nullptr) {
        m_dialogOtherVatRates = new DialogOtherVatRates(this);
        connect(m_dialogOtherVatRates,
                &DialogOtherVatRates::accepted,
                [this](){
            QString name = m_dialogOtherVatRates->getName();
            //auto currentSelection
                   //= ui->listOtherRates->selectionModel()->selectedRows();
            VatRateManager::instance()->addOtherRates(name);
            //if (currentSelection.size() > 0) {
                //ui->listOtherRates->selectionModel()->select(currentSelection.first());
            //}
        });
    }
    m_dialogOtherVatRates->clear();
    m_dialogOtherVatRates->show();
}
//----------------------------------------------------------
void PaneVatParams::removeSelectedOtherRates()
{
    auto selection = ui->listOtherRates->selectionModel()->selection();
    if (!selection.isEmpty()) {
        int ret = QMessageBox::warning(
                    this, tr("Suppression des taux de TVA sélectionnés"),
                    tr("Êtes-vous sûr de vouloir supprimer les taux de TVA sélectionnés ?\n"
                       "Cette action n'est pas réversible."),
                    QMessageBox::Ok | QMessageBox::Cancel);
        if (ret == QMessageBox::Ok) {
            int index = selection.indexes().first().row();
            VatRateManager::instance()->remove(index);
        }
    }
}
//----------------------------------------------------------
void PaneVatParams::removeSelectedSkus()
{
    auto selRates = ui->listOtherRates->selectionModel()->selection();
    if (selRates.isEmpty()) {
        QMessageBox::warning(
                    this, tr("Information"),
                    tr("Il faut d'abord sélectionner en ensemble de taux de TVA."),
                    QMessageBox::Ok | QMessageBox::Cancel);
    } else {
        QString ratesName = selRates.indexes().first().data().toString();
        auto model = VatRateManager::instance()->getSelectedSkusModel(ratesName);
        auto selection = ui->listSelectedSkus->selectionModel()->selectedRows();
        std::sort(selection.begin(), selection.end());
        for (auto it = selection.rbegin(); it != selection.rend(); ++it) {
            model->removeSku(it->data().toString());
        }
    }
}
//----------------------------------------------------------
void PaneVatParams::addSku()
{
    auto selection = ui->listOtherRates->selectionModel()->selection();
    if (selection.isEmpty()) {
        QMessageBox::warning(
                    this, tr("Information"),
                    tr("Il faut d'abord sélectionner en ensemble de taux de TVA."),
                    QMessageBox::Ok | QMessageBox::Cancel);
    } else {
        QString sku = QInputDialog::getText(
                    this,
                    tr("Entrez le code SKU"),
                    "SKU").trimmed();
        if (!sku.isEmpty()) {
            QString ratesName = selection.indexes().first().data().toString();
            auto model = VatRateManager::instance()->getSelectedSkusModel(ratesName);
            if (!model->addSku(sku)) {
                QMessageBox::warning(
                            this, tr("Erreur"),
                            tr("Le sku n'a pas été ajouté car il existe déjà."),
                            QMessageBox::Ok);
            }
        }
    }
}
//----------------------------------------------------------
void PaneVatParams::loadYearSkus()
{
    setCursor(Qt::WaitCursor);
    int year = ui->comboBoxYear->currentText().toInt();
    ///* /// To load faster, I can remove this
    VatOrdersModel::instance()->computeVat(
                year,
                    [](const Shipment *shipment) {}
                    , ""
                    , ""
                    , [](const Shipment *) -> bool {return false;}
                    , [](const Shipment *shipment) {
                auto articlesShipped = shipment->getArticlesShipped();
                for (auto itArt = articlesShipped.begin();
                 itArt != articlesShipped.end(); ++itArt) {
                    SkusFoundManager::instance()->add(itArt.value()->getSku());
                }}
    );
    setCursor(Qt::ArrowCursor);
}
//----------------------------------------------------------
void PaneVatParams::removeSkusFromRight()
{
    auto modelRight = ui->listSelectedSkus->model();
    auto selectedItems = ui->listSelectedSkus->selectionModel()->selectedRows();
    if (modelRight != nullptr && selectedItems.size() > 0) {
        SelectedSkusListModel *modelRightConv
                = dynamic_cast<SelectedSkusListModel *>(modelRight);
        for (auto item = selectedItems.begin();
             item != selectedItems.end(); ++item) {
            QString sku = item->data().toString();
            modelRightConv->removeSku(sku);
        }
    }
}
//----------------------------------------------------------
void PaneVatParams::addSkusToRight()
{
    //auto modelLeft = ui->listKnownSkus->model();
    //SkusFoundManager::instance()->
    auto modelRight = ui->listSelectedSkus->model();
    auto selectedItems = ui->listKnownSkus->selectionModel()->selectedRows();
    if (modelRight != nullptr && selectedItems.size() > 0) {
        SelectedSkusListModel *modelRightConv
                = dynamic_cast<SelectedSkusListModel *>(modelRight);
        for (auto item = selectedItems.begin();
             item != selectedItems.end(); ++item) {
            QString sku = item->data().toString();
            modelRightConv->addSku(sku);
        }
    }
}
//----------------------------------------------------------
void PaneVatParams::displaySelOtherRates(
        QItemSelection newSelection,
        QItemSelection previousSelection)
{
    if (newSelection.isEmpty()) {
        _hideOtherRatesFields();
        ui->listSelectedSkus->setModel(nullptr);
        ui->listKnownSkus->setModel(nullptr);
        ui->tableOtherRates->setModel(nullptr);
        ui->lineEditFilterKnownSkus->clear();
    } else {
        _showOtherRatesFields();
        (void) previousSelection;
        QString ratesName = newSelection.indexes().first().data().toString();
        auto modelSelSkus = VatRateManager::instance()->getSelectedSkusModel(ratesName);
        ui->listSelectedSkus->setModel(modelSelSkus.data());
        auto modelOtheRates = VatRateManager::instance()->getOtherVatModel(ratesName);
        ui->tableOtherRates->setModel(modelOtheRates.data());
    }
}
//----------------------------------------------------------

