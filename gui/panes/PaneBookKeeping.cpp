#include <qmessagebox.h>

#include "../common/utils/CsvHeader.h"

#include "PaneBookKeeping.h"
#include "ui_PaneBookKeeping.h"
#include "model/orderimporters/VatOrdersModel.h"
#include "model/bookkeeping/SettingBookKeeping.h"
#include "model/bookkeeping/entries/parsers/EntryParserPurchasesTable.h"
#include "model/bookkeeping/entries/parsers/EntryParserImportationsTable.h"
#include "model/bookkeeping/entries/parsers/ManagerEntryTables.h"
#include "model/bookkeeping/entries/parsers/EntryParserBankTable.h"
#include "model/bookkeeping/entries/parsers/EntryParserAmazonOrdersMonthly.h"
#include "model/bookkeeping/entries/parsers/EntryParserAmazonPaymentsTable.h"
#include "model/bookkeeping/entries/parsers/EntryParserAmazonPayments.h"
#include "model/bookkeeping/entries/parsers/TableEntryAssociations.h"
#include "model/bookkeeping/entries/AbstractEntrySaver.h"
#include "model/bookkeeping/entries/parsers/EntrySelfTable.h"
#include "model/bookkeeping/ExceptionAccountSaleMissing.h"
#include "model/bookkeeping/ExceptionAccountDeportedMissing.h"
#include "model/orderimporters/ImporterYearsManager.h"
#include "dialogs/DialogAddInvoicePurchase.h"
#include "dialogs/DialogAddInvoicePurchaseImport.h"
#include "dialogs/DialogAddInvoices.h"
#include "dialogs/DialogAddSelfEntry.h"
#include "dialogs/DialogAddBankAccount.h"
#include "dialogs/DialogBankBalances.h"

//----------------------------------------------------------
PaneBookKeeping::PaneBookKeeping(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneBookKeeping)
{
    ui->setupUi(this);
    ui->comboBoxYear->setModel(ImporterYearsManager::instance());
    m_dialogAddInvoice = nullptr;
    m_dialogAddInvoices = nullptr;
    m_dialogAddBankAccount = nullptr;
    m_dialogAddSelfEntry = nullptr;

    ui->tableNoConnections->setModel(
                EntrySelfTable::instance());
    ui->tableNoConnections->horizontalHeader()->resizeSection(0, 200);

    ui->tablePurchases->setModel(
                ManagerEntryTables::instance()->entryDisplayPurchase());
    ui->tablePurchases->setItemDelegate(
                new TableEntryPurchasesDelegate(
                    ManagerEntryTables::instance()->entryDisplayPurchase(),
                    ui->tablePurchases));
    ui->tablePurchases->setColumnHidden(0, true);
    ui->tablePurchases->horizontalHeader()->resizeSection(5, 150);

    initToolBoxBanks(
                ui->toolBoxBanks,
                ui->groupBox_3,
                ui->verticalSpacerDeleteShow);
    /*
    auto bankDisplay = ManagerEntryTables::instance()
            ->entryDisplayBanks();
    ui->toolBoxBanks->removeItem(0);
    ui->pageToDelete1->deleteLater();
    for (auto it = bankDisplay.begin();
         it != bankDisplay.end(); ++it) {
        auto tableView = new QTableView(ui->toolBoxBanks);
        tableView->setSortingEnabled(true);
        tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
        tableView->setSelectionMode(QAbstractItemView::MultiSelection);
        tableView->horizontalHeader()->setStretchLastSection(true);
        tableView->setModel(it.value());
        QString title = it.key();
        //ui->toolBoxBanks->addItem(tableView, it.key());
        tableView->horizontalHeader()->resizeSection(0, 150);
        tableView->horizontalHeader()->resizeSection(5, 150);
        tableView->setHidden(true);
        ui->toolBoxBanks->setHidden(true);
        connect(it.value(),
                &EntryParserBankTable::rowsInserted,
                [this, title, tableView](const QModelIndex &, int, int){
            if (tableView->model()->rowCount() > 0
                    && tableView->isHidden()) {
                ui->toolBoxBanks->addItem(tableView, title);
                ui->toolBoxBanks->setHidden(false);
                tableView->setHidden(false);
                ui->groupBox_3->layout()->removeItem(ui->verticalSpacerDeleteShow);
            }
        });
        connect(it.value(),
                &EntryParserBankTable::rowsRemoved,
                [this, title, tableView](const QModelIndex &, int, int){
            if (tableView->model()->rowCount() == 0) {
                int index = ui->toolBoxBanks->indexOf(tableView);
                ui->toolBoxBanks->removeItem(index);
                tableView->setHidden(true);
            }
        });
    }
    //*/

    auto salesDisplay = ManagerEntryTables::instance()
            ->entryDisplaySale();
    ui->toolBoxSales->removeItem(0);
    ui->pageToDelete2->deleteLater();
    for (auto it = salesDisplay.begin();
         it != salesDisplay.end(); ++it) {
        if (it.value()->displays()) {
            auto tableView = new QTableView(ui->toolBoxSales);
            tableView->setSortingEnabled(true);
            tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
            tableView->setSelectionMode(QAbstractItemView::MultiSelection);
            tableView->horizontalHeader()->setStretchLastSection(true);
            auto model = it.value();
            tableView->setModel(model);
            QString title = it.key();
            EntryParserAmazonPaymentsTable * modelConv
                    = dynamic_cast<EntryParserAmazonPaymentsTable *>(
                        model);
            if (modelConv != nullptr) {
                auto modelConv
                        = static_cast<EntryParserAmazonPaymentsTable *>(
                            model);
                tableView->setItemDelegate(
                            new EntryParserAmazonPaymentsDelegate(
                                modelConv, tableView));
            }
            ui->toolBoxSales->addItem(tableView, it.key());
        }
    }

    for (auto tableView : _allEntryParserTableViews()) {
        connect(tableView->selectionModel(),
                &QItemSelectionModel::selectionChanged,
                [tableView](const QItemSelection &newSelection,
                const QItemSelection &oldSelection) {
            if (newSelection.indexes().size() > 0) {
                int row = newSelection.indexes()[0].row();
                auto modelConv = static_cast<AbstractEntryParserTable *>(tableView->model());
                auto entrySet = modelConv->entrySet(row);
                TableEntryAssociations::instance()->selectAssociation(entrySet->id());
            } else if (oldSelection.indexes().size() > 0) {
                int row = oldSelection.indexes()[0].row();
                auto modelConv = static_cast<AbstractEntryParserTable *>(tableView->model());
                auto entrySet = modelConv->entrySet(row);
                TableEntryAssociations::instance()->unselectAssociation(entrySet->id());
            }
        });
    }

    _connectSlots();
}
//----------------------------------------------------------
void PaneBookKeeping::_connectSlots()
{
    connect(ui->buttonLoadYear,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::loadYearSelected);
    connect(ui->buttonGenBookKeeping,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::genBookKeeping);
    connect(ui->buttonCheckBankBalances,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::checkBankBalances);

    connect(ui->buttonAddSelfEntry,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::addSelfEntry);
    connect(ui->buttonRemoveSelfEntry,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::removeSelfEntry);
    connect(ui->buttonInternWire,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::internBankToBank);

    connect(ui->buttonAssociate,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::associate);
    connect(ui->buttonDissociate,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::dissociate);
    connect(ui->buttonUnselectAll,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::unselectAll);
    connect(ui->buttonHideAssociated,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::hideAssociated);

    connect(ui->buttonAddInvoice,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::addInvoicePurchase);
    connect(ui->buttonAddInvoices,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::addInvoicesPurchases);
    connect(ui->buttonRemoveInvoice,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::removeInvoicePurchase);
    connect(ui->buttonViewPurchaseFile,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::viewPurchaseFile);

    connect(ui->buttonAddInvoiceImport,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::addInvoicePurchaseImport);
    connect(ui->buttonRemoveInvoiceImport,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::removeInvoicePurchaseImport);

    connect(ui->buttonAddBankFile,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::addBankAccount);
    connect(ui->buttonRemoveBankFile,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::removeBankAccount);
    connect(ui->buttonViewBankFile,
            &QPushButton::clicked,
            this,
            &PaneBookKeeping::viewBankAccount);
}
//----------------------------------------------------------
PaneBookKeeping::~PaneBookKeeping()
{
    delete ui;
}
//----------------------------------------------------------
void PaneBookKeeping::initToolBoxBanks(
        QToolBox *toolBox,
        QGroupBox *groupBox,
        QSpacerItem *spacerToRemove)
{
    auto bankDisplay = ManagerEntryTables::instance()
            ->entryDisplayBanks();
    auto firstToolBoxItem = toolBox->widget(0);
    toolBox->removeItem(0);
    firstToolBoxItem->deleteLater();
    for (auto it = bankDisplay.begin();
         it != bankDisplay.end(); ++it) {
        auto tableView = new QTableView(toolBox);
        tableView->setSortingEnabled(true);
        tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
        tableView->setSelectionMode(QAbstractItemView::MultiSelection);
        tableView->horizontalHeader()->setStretchLastSection(true);
        tableView->setModel(it.value());
        QString title = it.key();
        //ui->toolBoxBanks->addItem(tableView, it.key());
        tableView->horizontalHeader()->resizeSection(0, 150);
        tableView->horizontalHeader()->resizeSection(5, 150);
        tableView->setHidden(true);
        toolBox->setHidden(true);
        connect(it.value(),
                &EntryParserBankTable::rowsInserted,
                [toolBox, groupBox, spacerToRemove,title, tableView](const QModelIndex &, int, int){
            if (tableView->model()->rowCount() > 0
                    && tableView->isHidden()) {
                toolBox->addItem(tableView, title);
                toolBox->setHidden(false);
                tableView->setHidden(false);
                groupBox->layout()->removeItem(spacerToRemove);
            }
        });
        connect(it.value(),
                &EntryParserBankTable::rowsRemoved,
                [toolBox, title, tableView](const QModelIndex &, int, int){
            if (tableView->model()->rowCount() == 0) {
                int index = toolBox->indexOf(tableView);
                toolBox->removeItem(index);
                tableView->setHidden(true);
            }
        });
    }
}
//----------------------------------------------------------
void PaneBookKeeping::genBookKeeping()
{
    if (!ui->comboBoxYear->currentText().isEmpty()) {
        setCursor(Qt::WaitCursor);
        int year = ui->comboBoxYear->currentText().toInt();
        auto entries = ManagerEntryTables::instance()->entries(year);
        auto saver = AbstractEntrySaver::selected();
        saver->save(entries, QDir(SettingBookKeeping::instance()->dirPath()));
        setCursor(Qt::ArrowCursor);
    }
    //TODO accounting dir from settings
}
//----------------------------------------------------------
void PaneBookKeeping::checkBankBalances()
{
    const QString &year = ui->comboBoxYear->currentText();
    DialogBankBalances dialog(year);
    dialog.exec();
}
//----------------------------------------------------------
void PaneBookKeeping::addInvoicePurchase()
{
    if (m_dialogAddInvoice == nullptr) {
        m_dialogAddInvoice = new DialogAddInvoicePurchase(this);
        connect(m_dialogAddInvoice,
                &DialogAddInvoicePurchase::accepted,
                [this](){
            auto info = m_dialogAddInvoice->getPurchaseInvoiceInfo();
            try {
                ManagerEntryTables::instance()->entryDisplayPurchase()
                        ->addInvoice(info);
            } catch(const EntryParserPurchasesException &exception) {
                QMessageBox::critical(
                            this, tr("Erreur"),
                            exception.error(),
                            QMessageBox::Ok);
            }
        });
    }
    m_dialogAddInvoice->clear();
    m_dialogAddInvoice->show();
}
//----------------------------------------------------------
void PaneBookKeeping::addInvoicesPurchases()
{
    if (m_dialogAddInvoices == nullptr) {
        m_dialogAddInvoices = new DialogAddInvoices(this);
        connect(m_dialogAddInvoices,
                &DialogAddInvoicePurchase::accepted,
                [this](){
            auto invoiceInfos = m_dialogAddInvoices->getPurchaseInvoiceInfos();
            try {
                for (auto itInv = invoiceInfos.begin();
                     itInv != invoiceInfos.end(); ++itInv) {
                    ManagerEntryTables::instance()->entryDisplayPurchase()
                            ->addInvoice(*itInv);
                }
            } catch(const EntryParserPurchasesException &exception) {
                QMessageBox::critical(
                            this, tr("Erreur"),
                            exception.error(),
                            QMessageBox::Ok);
            }
        });
    }
    m_dialogAddInvoices->clear();
    m_dialogAddInvoices->show();
}
//----------------------------------------------------------
void PaneBookKeeping::removeInvoicePurchase()
{
    auto rowIndexes = ui->tablePurchases->selectionModel()->selectedIndexes();
    QSet<int> rows;
    for (auto index : rowIndexes) {
        rows << index.row();
    }
    QList<int> rowsSorted = rows.toList();
    std::sort(rowsSorted.begin(), rowsSorted.end());
    while (rowsSorted.size() > 0) {
        int row = rowsSorted.takeLast();
        ManagerEntryTables::instance()->entryDisplayPurchase()
                ->removeEntry(row);
    }
}
//----------------------------------------------------------
void PaneBookKeeping::loadYearSelected()
{
    // TODO it should not crash when loading after having added files like invoice or bank file
    // TODO 2 I should load on customer change the table entry associations with a load function
    // that doesn't apply to entry set
    if (!ui->comboBoxYear->currentText().isEmpty()) {
        setCursor(Qt::WaitCursor);
        try {
            int year = ui->comboBoxYear->currentText().toInt();
            ///* /// To load faster, I can remove this
            VatOrdersModel::instance()->computeVat(
                        year,
                        [](const Shipment *shipment){
                for (auto entryDisplayTable : ManagerEntryTables::instance()->entryDisplaySale()) {
                    entryDisplayTable->saleParser()->recordTransactions(shipment);
                }
            });
            //*/
            ManagerEntryTables::instance()->load(year);
            TableEntryAssociations::instance()->loadAssociations(year);
        } catch (const ExceptionAccountSaleMissing &exception) {
            QMessageBox::warning(
                        this,
                        tr("Account missing"),
                        exception.accounts());
        } catch (const ExceptionAccountDeportedMissing &exception) {
            QMessageBox::warning(
                        this,
                        tr("Deported Account missing"),
                        exception.error());
        }
        setCursor(Qt::ArrowCursor);
    }
}
//----------------------------------------------------------
void PaneBookKeeping::unselectAll()
{
    ui->tableNoConnections->clearSelection();
    for (auto tableView : _allEntryParserTableViews()) {
        tableView->clearSelection();
    }
}
//----------------------------------------------------------
void PaneBookKeeping::addSelfEntry()
{
    if (m_dialogAddSelfEntry == nullptr) {
        m_dialogAddSelfEntry = new DialogAddSelfEntry(this);
        connect(m_dialogAddSelfEntry,
                &DialogAddSelfEntry::accepted,
                [this](){
            EntrySelfTable::instance()->addAccount(
                        m_dialogAddSelfEntry->title(),
                        m_dialogAddSelfEntry->account());
        });
    }
    m_dialogAddSelfEntry->clear();
    m_dialogAddSelfEntry->show();
}
//----------------------------------------------------------
void PaneBookKeeping::removeSelfEntry()
{
    auto rows = ui->tableNoConnections->selectionModel()->selectedRows();
    if (rows.size() > 0) {
        int row = rows[0].row();
        auto account = EntrySelfTable::instance()->account(row);
        EntrySelfTable::instance()->removeAccount(row);
        TableEntryAssociations::instance()->removeAllSelfAssociations(account.id);
    }
}
//----------------------------------------------------------
void PaneBookKeeping::internBankToBank()
{

}
//----------------------------------------------------------
void PaneBookKeeping::selfAssociate()
{
    auto rows = ui->tableNoConnections->selectionModel()->selectedRows();
    if (rows.size() > 0) {
        auto account = EntrySelfTable::instance()->account(rows[0].row());
        QList<QSharedPointer<AccountingEntrySet>> bankEntrySets;
        for (auto tableView : _allEntryParserTableViews()) {
            auto model = tableView->model();
            if (model != nullptr) {
                auto modelConv = static_cast<AbstractEntryParserTable *>(model);
                auto selIndexes = tableView->selectionModel()->selectedIndexes();
                QSet<int> selectedRows;
                for (auto sel : selIndexes) {
                    selectedRows << sel.row();
                }
                for (auto row : selectedRows) {
                    auto entrySet = modelConv->entrySet(row);
                    if (entrySet->type() == AccountingEntrySet::Bank
                            || entrySet->type() == AccountingEntrySet::Purchase) {
                        bankEntrySets << entrySet;
                    } else {
                        QMessageBox::critical(
                                    this, tr("Erreur"),
                                    tr("Pour un compte sans lettrage, vous devez sélectionner uniquement des lignes de banque ou facture d'achat."),
                                    QMessageBox::Ok);
                        return;
                    }
                }
            }
        }
        for (auto entrySet : bankEntrySets) {
            TableEntryAssociations::instance()->addSelfAssociation(
                        entrySet.data(), account.id, account.account, account.title);
            unselectAll();
        }
    } else {
        QMessageBox::critical(
                    this, tr("Erreur"),
                    tr("Vous devez sélectionner une ligne de banque et un compte sans lettrage."),
                    QMessageBox::Ok);
    }
}
//----------------------------------------------------------
void PaneBookKeeping::associate()
{
    auto rows = ui->tableNoConnections->selectionModel()->selectedRows();
    if (rows.size() > 0) {
        selfAssociate();
    } else {
        QList<QSharedPointer<AccountingEntrySet>> entrySetsPositive;
        QList<QSharedPointer<AccountingEntrySet>> entrySetsNegative;
        double sum = 0.;
        bool bankOnly = true;
        //bool hasMarketplacePayment = false;
        /// Getting bank currency which will be used for amazon payment later
        QSet<QString> currenciesBank;
        int nBanks = ui->toolBoxBanks->count();
        for (int i=0; i<nBanks; ++i) {
            auto item = ui->toolBoxBanks->widget(i);
            auto tableView = static_cast<QTableView *>(item);
            auto model = tableView->model();
            if (model != nullptr) {
                auto modelConv = static_cast<AbstractEntryParserTable *>(model);
                auto selIndexes = tableView->selectionModel()->selectedIndexes();
                QSet<int> selectedRows;
                for (auto sel : selIndexes) {
                    selectedRows << sel.row();
                }
                for (auto row : selectedRows) {
                    auto entrySet = modelConv->entrySet(row);
                    currenciesBank << entrySet->currencyOrig();
                }
            }
        }
        /// End getting bank currencies
        for (auto tableView : _allEntryParserTableViews()) {
            auto model = tableView->model();
            if (model != nullptr) {
                auto modelConv = static_cast<AbstractEntryParserTable *>(model);
                auto selIndexes = tableView->selectionModel()->selectedIndexes();
                QSet<int> selectedRows;
                for (auto sel : selIndexes) {
                    selectedRows << sel.row();
                }
                //auto selectedRows = tableView->selectionModel()->selectedRows();
                for (auto row : selectedRows) {
                    auto entrySet = modelConv->entrySet(row);
                    double amountOrig = entrySet->amountOrig();
                    if (entrySet->type() == AccountingEntrySet::SaleMarketplace
                            && (!currenciesBank.contains(entrySet->currencyOrig())
                                || currenciesBank.size() > 1)) {
                        amountOrig = entrySet->amountConv();
                        if (ManagerEntryTables::instance()
                                ->entryParserAmazonPayments()
                                ->isAmountPaidReplaced(entrySet->id())) {
                            amountOrig = ManagerEntryTables::instance()
                                    ->entryParserAmazonPayments()
                                    ->amountPaid(entrySet->id());
                        }
                    }
                    sum += amountOrig;
                    bankOnly &= entrySet->type() == AccountingEntrySet::Bank;
                    if (amountOrig > 0) {
                        entrySetsPositive << entrySet;
                    } else {
                        entrySetsNegative << entrySet;
                    }
                }
            }
        }
        if (entrySetsNegative.size() == 0 && entrySetsPositive.size() == 0)
        {
            return;
        }
        bankOnly &= (entrySetsNegative.size() == 1 && entrySetsPositive.size() == 1);
        if (bankOnly && entrySetsNegative[0]->currencyOrig() != entrySetsPositive[0]->currencyOrig()) {
            sum = 0;
            for (auto entrySet : entrySetsNegative) {
                sum += entrySet->amountConv();
            }
            for (auto entrySet : entrySetsPositive) {
                sum += entrySet->amountConv();
            }
        }
        double relDiffAfterConv = qAbs(1.*sum/entrySetsNegative[0]->amountOrig());
        if (qAbs(sum) < 0.009) { /// Associating
            bool debitContainsPurchase = false;
            for (auto entrySetDebit : entrySetsPositive) {
                for (auto entry : entrySetDebit->entries()) {
                    if (entry.account().startsWith("6")) {
                        debitContainsPurchase = true;
                        break;
                    }
                }
            }
            bool creditContainsPurchase = false;
            for (auto entrySetCredit : entrySetsNegative) {
                for (auto entry : entrySetCredit->entries()) {
                    if (entry.account().startsWith("6")) {
                        creditContainsPurchase = true;
                        break;
                    }
                }
            }
            if (debitContainsPurchase) {
                for (int i=entrySetsPositive.size() -1; i>=0; --i) {
                    bool has6 = false;
                    for (auto entry : entrySetsPositive[i]->entries()) {
                        if (entry.account().startsWith("6")) {
                            has6 = true;
                            break;
                        }
                    }
                    if (!has6) {
                        entrySetsNegative << entrySetsPositive[i];
                        entrySetsPositive.removeAt(i);
                    }
                }
            } else if (creditContainsPurchase) {
                for (int i=entrySetsNegative.size() -1; i>=0; --i) {
                    bool has6 = false;
                    for (auto entry : entrySetsNegative[i]->entries()) {
                        if (entry.account().startsWith("6")) {
                            has6 = true;
                            break;
                        }
                    }
                    if (!has6) {
                        entrySetsPositive << entrySetsNegative[i];
                        entrySetsNegative.removeAt(i);
                    }
                }
            }
            for (auto entrySetDebit : entrySetsPositive) {
                for (auto entrySetCredit : entrySetsNegative) {
                    int yearFrom = entrySetDebit->date().year();
                    int yearTo = entrySetCredit->date().year();
                    TableEntryAssociations::instance()->addAssociation(
                                yearFrom,
                                yearTo,
                                entrySetDebit.data(),
                                entrySetCredit.data());
                }
            }
            unselectAll();
        } else if (bankOnly
                   && entrySetsNegative[0]->currencyOrig() != entrySetsPositive[0]->currencyOrig()
                   && relDiffAfterConv < 0.09) { /// It means internal wire with small difference due to currency change
            auto entrySetDebit = entrySetsPositive[0];
            auto entrySetCredit = entrySetsNegative[0];
            int yearFrom = entrySetDebit->date().year();
            int yearTo = entrySetCredit->date().year();
            TableEntryAssociations::instance()->addAssociation(
                        yearFrom,
                        yearTo,
                        entrySetDebit.data(),
                        entrySetCredit.data());
            unselectAll();
        } else {
            QMessageBox::critical(
                        this, tr("Erreur"),
                        tr("La somme n'est pas nul. Il y a l'écart suivant: ")
                        + QString::number(sum, 'f', 2),
                        QMessageBox::Ok);
        }
    }
}
//----------------------------------------------------------
void PaneBookKeeping::dissociate()
{
    QList<QSharedPointer<AccountingEntrySet>> entrySetsSelfAssociated;
    QList<QSharedPointer<AccountingEntrySet>> entrySetsDebit;
    QList<QSharedPointer<AccountingEntrySet>> entrySetsCredit;
    for (auto tableView : _allEntryParserTableViews()) {
        auto model = tableView->model();
        if (model != nullptr) {
            auto modelConv = static_cast<AbstractEntryParserTable *>(model);
            auto selIndexes = tableView->selectionModel()->selectedIndexes();
            QSet<int> selectedRows;
            for (auto sel : selIndexes) {
                selectedRows << sel.row();
            }
            //auto selectedRows = tableView->selectionModel()->selectedRows();
            for (auto row : selectedRows) {
                auto entrySet = modelConv->entrySet(row);
                if (TableEntryAssociations::instance()->isSelfAssociated(entrySet->id())){
                    entrySetsSelfAssociated << entrySet;
                } else if (entrySet->isDebit()) {
                    entrySetsDebit << entrySet;
                } else {
                    entrySetsCredit << entrySet;
                }
            }
        }
    }
    for (auto entrySetDebit : entrySetsDebit) {
        TableEntryAssociations::instance()->removeAssociation(
                    entrySetDebit->date().year(),
                    entrySetDebit.data());
    }
    for (auto entrySetCredit : entrySetsCredit) {
        TableEntryAssociations::instance()->removeAssociation(
                    entrySetCredit->date().year(),
                    entrySetCredit.data());
    }
    for (auto entrySet : entrySetsSelfAssociated) {
        TableEntryAssociations::instance()->removeSelfAssociation(
                    entrySet.data());
    }
}
//----------------------------------------------------------
void PaneBookKeeping::hideAssociated(bool hide)
{
    for (auto tableView : _allEntryParserTableViews()) {
        auto model = tableView->model();
        if (model != nullptr) {
            auto modelConv = static_cast<AbstractEntryParserTable *>(model);
            int nRows = modelConv->rowCount();
            for (int i=0; i<nRows; ++i) {
                auto entrySet = modelConv->entrySet(i);
                bool isAssociated = TableEntryAssociations::instance()
                        ->isAssociated(entrySet->id());
                bool toHide = hide && isAssociated;
                tableView->setRowHidden(i, toHide);
            }
        }
    }
}
//----------------------------------------------------------
QList<QTableView *> PaneBookKeeping::_allEntryParserTableViews() const
{
    QList<QTableView *> tableViews;
    //tableViews << ui->tableNoConnections; //TODO
    tableViews << ui->tablePurchases;
    int nBanks = ui->toolBoxBanks->count();
    for (int i=0; i<nBanks; ++i) {
        auto item = ui->toolBoxBanks->widget(i);
        tableViews << static_cast<QTableView *>(item);
    }

    int nSales = ui->toolBoxSales->count();
    for (int i=0; i<nSales; ++i) {
        auto item = ui->toolBoxSales->widget(i);
        tableViews << static_cast<QTableView *>(item);
    }
    return tableViews;
}
//----------------------------------------------------------
void PaneBookKeeping::viewPurchaseFile()
{
    //TODO dialog that show file of purchase entry
}
//----------------------------------------------------------
void PaneBookKeeping::addInvoicePurchaseImport()
{
    DialogAddInvoicePurchaseImport dialogInvoiceImport;
    dialogInvoiceImport.exec();
    if (dialogInvoiceImport.wasAccepted()) {
        auto info = dialogInvoiceImport.getImportInvoiceInfo();
        try {
            ManagerEntryTables::instance()->entryDisplayImportations()
                    ->addInvoice(info);
        } catch(const EntryParserPurchasesException &exception) {
            QMessageBox::critical(
                        this, tr("Erreur"),
                        exception.error(),
                        QMessageBox::Ok);
        }
    }
}
//----------------------------------------------------------
void PaneBookKeeping::removeInvoicePurchaseImport()
{
    auto rowIndexes = ui->tablePurchases
            ->selectionModel()->selectedIndexes();
    QSet<int> rows;
    for (auto index : rowIndexes) {
        rows << index.row();
    }
    QList<int> rowsSorted = rows.toList();
    std::sort(rowsSorted.begin(), rowsSorted.end());
    while (rowsSorted.size() > 0) {
        int row = rowsSorted.takeLast();
        ManagerEntryTables::instance()
                ->entryDisplayImportations()
                ->removeEntry(row);
    }

}
//----------------------------------------------------------
void PaneBookKeeping::addBankAccount()
{
    if (m_dialogAddBankAccount == nullptr) {
        m_dialogAddBankAccount = new DialogAddBankAccount(this);
        connect(m_dialogAddBankAccount,
                &DialogAddBankAccount::accepted,
                [this](){
            QString bank = m_dialogAddBankAccount->getBank();
            QDate date = m_dialogAddBankAccount->getDate();
            QString filePath = m_dialogAddBankAccount->getBankFilePath();
            QString filePathDisplay = m_dialogAddBankAccount->getBankFilePathDisplay();
            try {
                ManagerEntryTables::instance()->entryDisplayBanks()[bank]
                        ->addFilePath(date.year(),
                                      date.month(),
                                      filePath,
                                      filePathDisplay);
            } catch(const EntryParserBankException &exception) {
                QMessageBox::critical(
                            this, tr("Erreur"),
                            exception.error(),
                            QMessageBox::Ok);
            } catch(const CsvHeaderException &exception) {
                QMessageBox::critical(
                            this, tr("Erreur dans l'entête"),
                            tr("L'entête du fichier ne contient pas l'une de ces colonnes ")
                            + exception.columnValuesError().join(" ,"),
                            QMessageBox::Ok);
            }
        });
    }
    m_dialogAddBankAccount->clear();
    m_dialogAddBankAccount->show();
}
//----------------------------------------------------------
void PaneBookKeeping::removeBankAccount()
{

}
//----------------------------------------------------------
void PaneBookKeeping::viewBankAccount()
{

}
//----------------------------------------------------------

