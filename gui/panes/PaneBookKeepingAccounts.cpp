#include <QtCore/qdir.h>
#include <QtCore/qsettings.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/QDoubleSpinBox>
#include <qfiledialog.h>

#include "../common/countries/CountryManager.h"

#include "PaneBookKeepingAccounts.h"
#include "model/bookkeeping/SettingBookKeeping.h"
#include "model/bookkeeping/ManagerAccountsAmazon.h"
#include "model/bookkeeping/ManagerAccountsSales.h"
#include "model/bookkeeping/ManagerSaleTypes.h"
//#include "model/bookkeeping/ManagerAccountsSalesRares.h"
#include "model/bookkeeping/ManagerAccountPurchase.h"
#include "model/bookkeeping/ManagerAccountsVatPayments.h"
#include "model/bookkeeping/ManagerAccountsStockDeported.h"
#include "model/bookkeeping/entries/parsers/ManagerEntryTables.h"
#include "model/bookkeeping/entries/AbstractEntrySaver.h"
#include "model/orderimporters/Shipment.h"
#include "gui/panes/dialogs/DialogAddAccountAmazon.h"
#include "ui_PaneBookKeepingAccounts.h"
#include "dialogs/DialogAddVatSaleAccount.h"

/// TODO create delegae + exection to check edited values no duplicate
//----------------------------------------------------------
PaneBookKeepingAccounts::PaneBookKeepingAccounts(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneBookKeepingAccounts), UpdateToCustomer()
{
    ui->setupUi(this);
    ui->comboBoxBookKeepingSaving->addItems(
                AbstractEntrySaver::entrySaverNames());
    ui->tableAccountsAmazon->setModel(
                ManagerAccountsAmazon::instance());
    ui->tableAccountsSalesUE->setModel(
                ManagerAccountsSales::instance());
    //ui->tableAccountsSalesRares->setModel(
                //ManagerAccountsSalesRares::instance());
    ui->tableAccountsPurchases->setModel(
                ManagerAccountPurchase::instance());
    ui->tableJournaux->setModel(
                ManagerEntryTables::instance());
    ui->tableAccountsVatPayments->setModel(
                ManagerAccountsVatPayments::instance());
    ui->tableAccountsStockDeported->setModel(
                ManagerAccountsStockDeported::instance());
    auto delegate = new DelegateAccountsSalesUE(this);
    ui->tableAccountsSalesUE->setItemDelegate(
                delegate);
    m_dialogAddAccountAmazon = nullptr;
    init();
    ui->lineEditStockDeportedAccount4->setText(
                ManagerAccountsStockDeported::instance()->getAccount4());
    ui->lineEditStockDeportedAccount4VatToPay->setText(
                ManagerAccountsStockDeported::instance()->getAccount4VatToPay());
    ui->lineEditStockDeportedAccount4vatDeductible->setText(
                ManagerAccountsStockDeported::instance()->getAccount4VatDeductible());
    ui->lineEditStockDeportedAccount4OutsideCountry->setText(
                ManagerAccountsStockDeported::instance()->getAccount4OutsideCountry());
    _connectSlots();
}
//----------------------------------------------------------
void PaneBookKeepingAccounts::onCustomerSelectedChanged(
        const QString &customerId)
{
    ui->lineEditBookKeeping->setText(
                SettingBookKeeping::instance()->dirPath());
    ui->comboBoxBookKeepingSaving->setCurrentText(
                SettingBookKeeping::instance()->saverName());
    ui->lineEditStockDeportedAccount4->setText(
                ManagerAccountsStockDeported::instance()->getAccount4());
    ui->lineEditStockDeportedAccount4VatToPay->setText(
                ManagerAccountsStockDeported::instance()->getAccount4VatToPay());
    ui->lineEditStockDeportedAccount4vatDeductible->setText(
                ManagerAccountsStockDeported::instance()->getAccount4VatDeductible());
    ui->lineEditStockDeportedAccount4OutsideCountry->setText(
                ManagerAccountsStockDeported::instance()->getAccount4OutsideCountry());
}
//----------------------------------------------------------
void PaneBookKeepingAccounts::_connectSlots()
{
    connect(ui->lineEditStockDeportedAccount4,
            &QLineEdit::textEdited,
            ManagerAccountsStockDeported::instance(),
            &ManagerAccountsStockDeported::setAccount4);
    connect(ui->lineEditStockDeportedAccount4OutsideCountry,
            &QLineEdit::textEdited,
            ManagerAccountsStockDeported::instance(),
            &ManagerAccountsStockDeported::setAccount4OutsideCountry);
    connect(ui->lineEditStockDeportedAccount4VatToPay,
            &QLineEdit::textEdited,
            ManagerAccountsStockDeported::instance(),
            &ManagerAccountsStockDeported::setAccount4VatToPay);
    connect(ui->lineEditStockDeportedAccount4vatDeductible,
            &QLineEdit::textEdited,
            ManagerAccountsStockDeported::instance(),
            &ManagerAccountsStockDeported::setAccount4VatDeductible);
    connect(ui->buttonAddAmazon,
            &QPushButton::clicked,
            this,
            &PaneBookKeepingAccounts::addAccountAmazon);
    connect(ui->buttonRemoveAmazon,
            &QPushButton::clicked,
            this,
            &PaneBookKeepingAccounts::removeAccountAmazon);
    connect(ui->buttonAddVatConf,
            &QPushButton::clicked,
            this,
            &PaneBookKeepingAccounts::addAccountVatConf);
    connect(ui->buttonRemoveVatConf,
            &QPushButton::clicked,
            this,
        &PaneBookKeepingAccounts::removeAccountVatConf);
    connect(ui->buttonDirBookKeeping,
        &QPushButton::clicked,
        this,
        &PaneBookKeepingAccounts::browseBookKeepingDir);
    connect(ui->comboBoxBookKeepingSaving,
        &QComboBox::currentTextChanged,
        [](const QString &saver) {
        SettingBookKeeping::instance()->setSaverName(saver);
    });
}
//----------------------------------------------------------
PaneBookKeepingAccounts::~PaneBookKeepingAccounts()
{
    delete ui;
}
//----------------------------------------------------------
QString PaneBookKeepingAccounts::uniqueId() const
{
    return "PaneBookKeepingAccounts";
}
//----------------------------------------------------------
void PaneBookKeepingAccounts::browseBookKeepingDir()
{
    QSettings settings;
    QString dirPath = QFileDialog::getExistingDirectory(
                this, tr("Choisir un répertoire"),
                SettingBookKeeping::instance()->dirPath());
    if (!dirPath.isEmpty()) {
        SettingBookKeeping::instance()->setDirPath(dirPath);
    }
    ui->lineEditBookKeeping->setText(dirPath);
}
//----------------------------------------------------------
void PaneBookKeepingAccounts::addAccountAmazon()
{
    if (m_dialogAddAccountAmazon == nullptr) {
        m_dialogAddAccountAmazon = new DialogAddAccountAmazon(this);
        connect(m_dialogAddAccountAmazon,
            &DialogAddAccountAmazon::accepted,
            this,
            [this](){
            QString amazon = m_dialogAddAccountAmazon->getAmazon();
            QString FAMAZON = m_dialogAddAccountAmazon->getFAMAZON();
            QString accountCustomer = m_dialogAddAccountAmazon->getAccountCustomer();
            QString accountReserve = m_dialogAddAccountAmazon->getAccountReserve();
            QString accountUnknownSales = m_dialogAddAccountAmazon->getAccountUnknownSales();
            QString accountCharges = m_dialogAddAccountAmazon->getAccountCharge();
            ManagerAccountsAmazon::instance()->
                    addAmazon(amazon,
                              accountCustomer,
                              accountReserve,
                              accountUnknownSales,
                              FAMAZON,
                              accountCharges);
        });
    } else {
        m_dialogAddAccountAmazon->clear();
    }
    m_dialogAddAccountAmazon->show();
}
//----------------------------------------------------------
void PaneBookKeepingAccounts::removeAccountAmazon()
{
    auto selectedIndexes = ui->tableAccountsAmazon
            ->selectionModel()->selectedIndexes();
    if (selectedIndexes.size() > 0) {
        ManagerAccountsAmazon::instance()->remove(
                    selectedIndexes.first());
    }
    /*
    QModelIndexList uniqueIndexes;
    QSet<int> rowToRemove;
    for (auto index : selectedIndexes) {
        if (!rowToRemove.contains(index.row())) {
            rowToRemove << index.row();
            uniqueIndexes << index;
        }
    }
    QList<int> rowToRemoveSort = rowToRemove.toList();
    std::sort(rowToRemoveSort.begin(), rowToRemoveSort.end());
    for (auto it = rowToRemoveSort.rbegin();
         it != rowToRemoveSort.rend();
         ++it) {
        ManagerAccountsAmazon::instance()->remove(*it);
    }
    //*/
}
//----------------------------------------------------------
void PaneBookKeepingAccounts::addAccountVatConf()
{
    DialogAddVatSaleAccount dialog;
    dialog.exec();
    if (dialog.wasAccepted()) {
        ManagerAccountsSales::instance()->addRow(
                    dialog.getRegime(),
                    dialog.getCountryName(),
                    dialog.getSaleType(),
                    dialog.getVatRateString(),
                    dialog.getAccountSale(),
                    dialog.getAccountVat());
    }
}
//----------------------------------------------------------
void PaneBookKeepingAccounts::removeAccountVatConf()
{
    auto selectedIndexes = ui->tableAccountsSalesUE
            ->selectionModel()->selectedIndexes();
    QSet<int> rowToRemove;
    for (auto index : selectedIndexes) {
        rowToRemove << index.row();
    }
    QList<int> rowToRemoveSort = rowToRemove.toList();
    std::sort(rowToRemoveSort.begin(), rowToRemoveSort.end());
    for (auto it = rowToRemoveSort.rbegin();
         it != rowToRemoveSort.rend();
         ++it) {
        ManagerAccountsSales::instance()->removeRow(*it);
    }
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
DelegateAccountsSalesUE::DelegateAccountsSalesUE(QObject *parent)
    : QStyledItemDelegate(parent)
{
}
//----------------------------------------------------------
//----------------------------------------------------------
QWidget *DelegateAccountsSalesUE::createEditor(
        QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    QWidget *widget = nullptr;
    if (index.column() == 0) {
        QComboBox *comboBox = new QComboBox(parent);
        comboBox->addItems(*Shipment::allRegimes());
        widget = comboBox;
    } else if (index.column() == 1) {
        QComboBox *comboBox = new QComboBox(parent);
        comboBox->addItems(*CountryManager::countriesNamesUEfrom2020());
        widget = comboBox;
    } else if (index.column() == 2) {
        QComboBox *comboBox = new QComboBox(parent);
        comboBox->addItems(
        {ManagerSaleTypes::SALE_PRODUCTS,
         ManagerSaleTypes::SALE_SERVICES,
         ManagerSaleTypes::SALE_PAYMENT_FASCILITOR});
        widget = comboBox;
    } else if (index.column() == 3) {
        QDoubleSpinBox *spinBox = new QDoubleSpinBox(parent);
        spinBox->setRange(0, 99);
        spinBox->setDecimals(4);
        spinBox->setSingleStep(0.01);
        widget = spinBox;
    } else {
        widget = QStyledItemDelegate::createEditor(
                    parent, option, index);
    }
    return widget;
}
//----------------------------------------------------------
//----------------------------------------------------------
void DelegateAccountsSalesUE::setEditorData(
        QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == 0
            || index.column() == 1
            || index.column() == 2) {
        QComboBox *comboBox = static_cast<QComboBox *>(editor);
        comboBox->setCurrentText(index.data().toString());
    } else if (index.column() == 3) {
        QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox *>(editor);
        spinBox->setValue(index.data().toString().toDouble());
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}
//----------------------------------------------------------
//----------------------------------------------------------
