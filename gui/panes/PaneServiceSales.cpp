#include <QMessageBox>

#include "dialogs/DialogAddShippingAddress.h"
#include "dialogs/DialogAddServiceSale.h"
#include "model/bookkeeping/entries/parsers/ManagerEntryTables.h"
#include "model/bookkeeping/entries/parsers/EntryParserBankTable.h"
#include "model/orderimporters/ImporterYearsManager.h"
#include "model/orderimporters/AddressesServiceCustomer.h"
#include "model/orderimporters/ServiceSalesModel.h"

#include "PaneServiceSales.h"
#include "ui_PaneServiceSales.h"

//----------------------------------------------------------
PaneServiceSales::PaneServiceSales(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneServiceSales)
{
    ui->setupUi(this);
    ui->comboBoxYear->setModel(ImporterYearsManager::instance());
    ui->listViewAffiliates->setModel(
                AddressesServiceCustomer::instance());
    ui->tableViewSales->setModel(ServiceSalesModel::instance());
    PaneBookKeeping::initToolBoxBanks(
                ui->toolBoxBanks,
                ui->groupBoxBanks,
                ui->verticalSpacerDeleteShow);
    _connectSlots();
}
//----------------------------------------------------------
PaneServiceSales::~PaneServiceSales()
{
    delete ui;
}
//----------------------------------------------------------
void PaneServiceSales::loadYearSelected()
{
    if (!ui->comboBoxYear->currentText().isEmpty()) {
        int year = ui->comboBoxYear->currentText().toInt();
        QMap<QString, EntryParserBankTable*> banks
                = ManagerEntryTables::instance()->entryDisplayBanks();
        for (auto itBank = banks.begin();
             itBank != banks.end(); ++itBank) {
            itBank.value()->load(year);
        }
        ServiceSalesModel::instance()->load(year);
    }
}
//----------------------------------------------------------
void PaneServiceSales::addBankAccount()
{
    QMessageBox::information(
                this,
                tr("Ajout de compte bancaire"),
                tr("Il faut utiliser l’onglet comptabliité"));
}
//----------------------------------------------------------
void PaneServiceSales::removeBankAccount()
{
    QMessageBox::information(
                this,
                tr("Supprimer un compte bancaire"),
                tr("Il faut utiliser l’onglet comptabliité"));
}
//----------------------------------------------------------
void PaneServiceSales::viewBankAccount()
{
    QMessageBox::information(
                this,
                tr("Visualiser un compte bancaire"),
                tr("Il faut utiliser l’onglet comptabliité"));
}
//----------------------------------------------------------
void PaneServiceSales::addAddress()
{
    DialogAddShippingAddress dialog;
    dialog.exec();
    if (dialog.wasAccepted()) {
        Address address = dialog.getAddress();
        AddressesServiceCustomer::instance()->addAddress(address);
    }
}
//----------------------------------------------------------
void PaneServiceSales::removeAddress()
{
    int ret = QMessageBox::warning(
                this, tr("Suppression de l'adresse sélectionné"),
                tr("Êtes-vous sûr de vouloir supprimer l'adresse séléctionnée ?\n"
                   "Cette action n'est pas réversible.\n"
                   "Si cette adresse était utilisée, elle sera remplacée par celle par défaut."),
                QMessageBox::Ok | QMessageBox::Cancel);
    if (ret == QMessageBox::Ok) {
        auto indexes = ui->listViewAffiliates->selectionModel()->selectedRows();
        std::sort(indexes.begin(), indexes.end());
        for (auto it = indexes.rbegin(); it != indexes.rend(); ++it) {
            AddressesServiceCustomer::instance()->removeAddress(it->row());
        }
    }
}
//----------------------------------------------------------
void PaneServiceSales::saveAddress() const
{
    auto indexes = ui->listViewAffiliates->selectionModel()->selectedRows();
    if (indexes.size() > 0) {
        saveAddressInPosition(indexes.first().row());
    }
}
//----------------------------------------------------------
void PaneServiceSales::saveAddressInPosition(int index) const
{
    Address address = ui->widgetAddress->getAddress();
    AddressesServiceCustomer::instance()->updateAddress(index, address);
}
//----------------------------------------------------------
void PaneServiceSales::displayAddress(
        QItemSelection newSelection,
        QItemSelection previousSelection)
{
    if (newSelection.isEmpty()) {
        ui->widgetAddress->hide();
    } else {
        if (!previousSelection.isEmpty()) {
            Address editedAddress = ui->widgetAddress->getAddress();
            int indexPrevious = previousSelection.indexes().first().row();
            Address previousAddress
                    = AddressesServiceCustomer::instance()->getAddress(indexPrevious);
            if (editedAddress != previousAddress) {
                int ret = QMessageBox::warning(this, tr("Sauvegarder les modifications"),
                                               tr("Avant de changer d'adresse, souhaitez-vous\n"
                                                  "sauvegarder l'addresse ?"),
                                               QMessageBox::Ok | QMessageBox::No);
                if (ret == QMessageBox::Ok) {
                    saveAddressInPosition(indexPrevious);
                }
            }
        }
        int indexNew = newSelection.indexes().first().row();
        Address address = AddressesServiceCustomer::instance()->getAddress(indexNew);
        ui->widgetAddress->setAddress(address);
        ui->widgetAddress->show();
        ui->widgetAddressAccounts->show();
        m_serviceAccounts = QSharedPointer<ServiceAccounts>(
                    new ServiceAccounts(address.internalId()));
        ui->lineEditAccountSale->setText(
                    m_serviceAccounts->accountSale());
        ui->lineEditAccountVatToDeclare->setText(
                    m_serviceAccounts->accountVatToDeclare());
        ui->lineEditAccountVatCollected->setText(
                    m_serviceAccounts->accountVatCollected());
        ui->lineEditAccountClient->setText(
                    m_serviceAccounts->accountClient());
    }
}
//----------------------------------------------------------
void PaneServiceSales::addSale()
{
    QString customer = _setSelServiceCustomer();
    if (customer.isEmpty()) {
        QMessageBox::information(
                    this,
                    tr("Pas de client"),
                    tr("Vous devez sélectionner un client"));
    } else {
        QString addressId = _setSelServiceCustomerAddressId();
        DialogAddServiceSale dialog;
        dialog.exec();
        if (dialog.wasAccepted()) {
            ServiceSalesModel::instance()->addSale(
                        dialog.getDate(),
                        dialog.getReference(),
                        dialog.getAmountUnit(),
                        dialog.getUnits(),
                        dialog.getTitle(),
                        dialog.getCurrency(),
                        customer,
                        addressId);
        }
    }
}
//----------------------------------------------------------
void PaneServiceSales::addSaleFromBank()
{
    QString customer = _setSelServiceCustomer();
    if (customer.isEmpty()) {
        QMessageBox::information(
                    this,
                    tr("Pas de client"),
                    tr("Vous devez sélectionner un client"));
    } else {
        int nBanks = ui->toolBoxBanks->count();
        for (int i=0; i<nBanks; ++i) {
            auto item = ui->toolBoxBanks->widget(i);
            auto tableView = static_cast<QTableView *>(item);
            auto model = tableView->model();
            if (model != nullptr && tableView->isVisible()) {
                auto modelConv = static_cast<AbstractEntryParserTable *>(model);
                auto selIndexes = tableView->selectionModel()->selectedIndexes();
                if (selIndexes.size() > 0) {
                    int row = selIndexes.first().row();
                    auto entrySet = modelConv->entrySet(row);
                    QString currency = entrySet->currencyOrig();
                    QString reference = entrySet->label().split(" ")[0];
                    double amount = entrySet->amountOrig();
                    auto date = entrySet->date();
                    DialogAddServiceSale dialog;
                    dialog.init(date,
                                reference,
                                amount,
                                currency);
                    dialog.exec();
                    if (dialog.wasAccepted()) {
                        QString addressId = _setSelServiceCustomerAddressId();
                        ServiceSalesModel::instance()->addSale(
                                    dialog.getDate(),
                                    dialog.getReference(),
                                    dialog.getAmountUnit(),
                                    dialog.getUnits(),
                                    dialog.getTitle(),
                                    dialog.getCurrency(),
                                    customer,
                                    addressId);
                    }
                    /*
                    SelfInvoiceSales::instance()->addSale(
                                date,
                                reference,
                                amount,
                                currency,
                                customer);
                                //*/
                }
            }
        }
    }
}
//----------------------------------------------------------
void PaneServiceSales::removeSale()
{
    auto selIndexes = ui->tableViewSales->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0) {
        ServiceSalesModel::instance()->remove(selIndexes.first());
    }
}
//----------------------------------------------------------
QString PaneServiceSales::_setSelServiceCustomer() const
{
    auto selIndexes = ui->listViewAffiliates
            ->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0) {
        return selIndexes.first().data().toString();
    }
    return QString();
}
//----------------------------------------------------------
QString PaneServiceSales::_setSelServiceCustomerAddressId() const
{
    auto selIndexes = ui->listViewAffiliates
            ->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0) {
        int rowIndex = selIndexes.first().row();
        return AddressesServiceCustomer::instance()->getAddress(rowIndex).internalId();
    }
    return QString();
}
//----------------------------------------------------------
void PaneServiceSales::_connectSlots()
{
    connect(ui->buttonLoadYear,
            &QPushButton::clicked,
            this,
            &PaneServiceSales::loadYearSelected);
    connect(ui->buttonAddBankFile,
            &QPushButton::clicked,
            this,
            &PaneServiceSales::addBankAccount);
    connect(ui->buttonRemoveBankFile,
            &QPushButton::clicked,
            this,
            &PaneServiceSales::removeBankAccount);
    connect(ui->buttonViewBankFile,
            &QPushButton::clicked,
            this,
            &PaneServiceSales::viewBankAccount);
    connect(ui->buttonAdd,
            &QPushButton::clicked,
            this,
            &PaneServiceSales::addAddress);
    connect(ui->buttonRemove,
            &QPushButton::clicked,
            this,
            &PaneServiceSales::removeAddress);
    connect(ui->buttonSave,
            &QPushButton::clicked,
            this,
            &PaneServiceSales::saveAddress);
    connect(ui->listViewAffiliates->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &PaneServiceSales::displayAddress);
    connect(ui->buttonAddSale,
            &QPushButton::clicked,
            this,
            &PaneServiceSales::addSale);
    connect(ui->buttonAddSaleFromBank,
            &QPushButton::clicked,
            this,
            &PaneServiceSales::addSaleFromBank);
    connect(ui->buttonRemoveSale,
            &QPushButton::clicked,
            this,
            &PaneServiceSales::removeSale);
    connect(ui->lineEditAccountSale,
            &QLineEdit::textEdited,
            this,
            [this](const QString &text){
        if (!m_serviceAccounts.isNull()) {
            m_serviceAccounts->setAccountSale(text);
        }
    });
    connect(ui->lineEditAccountVatCollected,
            &QLineEdit::textEdited,
            this,
            [this](const QString &text){
        if (!m_serviceAccounts.isNull()) {
            m_serviceAccounts->setAccountVatCollected(text);
        }
    });
    connect(ui->lineEditAccountClient,
            &QLineEdit::textEdited,
            this,
            [this](const QString &text){
        if (!m_serviceAccounts.isNull()) {
            m_serviceAccounts->setAccountClient(text);
        }
    });
    connect(ui->lineEditAccountVatToDeclare,
            &QLineEdit::textEdited,
            this,
            [this](const QString &text){
        if (!m_serviceAccounts.isNull()) {
            m_serviceAccounts->setAccountVatToDeclare(text);
        }
    });
}
//----------------------------------------------------------
