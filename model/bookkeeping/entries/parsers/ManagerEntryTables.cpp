#include <QSettings>

#include "model/SettingManager.h"
#include "ManagerEntryTables.h"
#include "AbstractEntryParserTable.h"
#include "EntryParserBankTable.h"
#include "EntryParserPurchasesTable.h"
#include "EntryParserImportationsTable.h"
#include "EntryParserBankQontoTable.h"
#include "EntryParserBankWiseEURTable.h"
#include "EntryParserBankWiseUSDTable.h"
#include "EntryParserBankWiseGBPTable.h"
#include "EntryParserBankPaypalEURTable.h"
#include "EntryParserBankPaypalUSDTable.h"
#include "EntryParserBankStripeEURTable.h"
#include "EntryParserBankStripeUSDTable.h"
#include "EntryParserAmazonOrdersMonthlyTable.h"
#include "EntryParserCdiscountPaymentTable.h"
#include "EntryParserCdiscountMonthlyTable.h"
#include "EntryParserFnacMonthlyTable.h"
#include "EntryParserAmazonPaymentsTable.h"
#include "EntryParserAmazonPayments.h"
#include "EntryParserServiceOrdersTable.h"
#include "EntryParserOrdersCustomTable.h"
#include "EntryParserSaleVatOssIossTable.h"
#include "EntryParserStockDeportedTable.h"

//----------------------------------------------------------
ManagerEntryTables *ManagerEntryTables::instance()
{
    static ManagerEntryTables instance;
    return &instance;
}
//----------------------------------------------------------
ManagerEntryTables::~ManagerEntryTables()
{

}
//----------------------------------------------------------
void ManagerEntryTables::onCustomerSelectedChanged(
            const QString &customerId)
{

}
//----------------------------------------------------------
QString ManagerEntryTables::uniqueId() const
{
    return "ManagerEntryTables";
}
//----------------------------------------------------------
void ManagerEntryTables::_clear()
{
      beginRemoveRows(QModelIndex(), 0, rowCount()-1);
      m_journalNames.clear();
      m_journalNamesHash.clear();
      endRemoveRows();
}
//----------------------------------------------------------
ManagerEntryTables::ManagerEntryTables(QObject *parent)
    : QAbstractTableModel(parent), UpdateToCustomer ()
{
    m_entryDisplayPurchase = new EntryParserPurchasesTable(this);
    m_entryDisplayImportation = new EntryParserImportationsTable(this);
    m_journalNames << QStringList({m_entryDisplayPurchase->name(),
                                   m_entryDisplayPurchase->journal()});;

    QList<EntryParserBankTable *> banks;
    banks << new EntryParserBankQontoTable(this);
    banks << new EntryParserBankWiseEURTable(this);
    banks << new EntryParserBankWiseUSDTable(this);
    banks << new EntryParserBankWiseGBPTable(this);
    banks << new EntryParserBankPaypalEURTable(this);
    banks << new EntryParserBankPaypalUSDTable(this);
    banks << new EntryParserBankStripeEURTable(this);
    banks << new EntryParserBankStripeUSDTable(this);
    for (auto bank : qAsConst(banks)) {
        m_entryDisplayBanks[bank->name()] = bank;
        m_journalNames << QStringList({bank->name(), bank->journal()});;
    }

    QList<EntryParserOrdersTable *> sales;
    sales << new EntryParserAmazonOrdersMonthlyTable(this);
    sales << new EntryParserCdiscountMonthlyTable(this);
    sales << new EntryParserCdiscountPaymentTable(this);
    sales << new EntryParserFnacMonthlyTable(this);
    sales << new EntryParserAmazonPaymentsTable(this);
    sales << new EntryParserServiceOrdersTable(this);
    sales << new EntryParserOrdersCustomTable(this);
    sales << new EntryParserSaleVatOssIossTable(this);
    for (auto sale : qAsConst(sales)) {
        QString name = sale->name();
        Q_ASSERT(!m_entryDisplaySales.contains(name));
        m_entryDisplaySales[name] = sale;
        m_journalNames << QStringList({name, sale->journal()});;
    }

    auto entryParserStockDeported = new EntryParserStockDeportedTable(this);
    QString name = entryParserStockDeported->name();
    m_entryDisplayVarious[name] = entryParserStockDeported;
    m_journalNames << QStringList({name, entryParserStockDeported->journal()});;

    for (auto line : qAsConst(m_journalNames)) {
        m_journalNamesHash[line[0]] = line[1];
    }
    loadFromSettings();
    init();
}
//----------------------------------------------------------
void ManagerEntryTables::saveFromSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!settingKey().isEmpty()) {
        QStringList lines;
        for (auto it=m_journalNames.begin();
             it!=m_journalNames.end();
             ++it) {
            QStringList elements
                    = {it->value(0), it->value(1)};
            lines << elements.join(";;;");
        }
        settings.setValue(settingKey(), lines.join(":::"));
    }
}
//----------------------------------------------------------
void ManagerEntryTables::loadFromSettings()
{
    //_clear();
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!settingKey().isEmpty()) {
        if (settings.contains(settingKey())) {
            QString text = settings.value(settingKey()).toString();
            QStringList lines = text.split(":::");
            //m_journalNamesHash.clear();
            for (auto line:lines) {
                QStringList elements = line.split(";;;");
                m_journalNamesHash[elements[0]] = elements[1];
            }
            for (int i=0; i<m_journalNames.size(); ++i) {
                QString valFromSettings = m_journalNamesHash.value(
                            m_journalNames[i][0], m_journalNames[i][1]);
                m_journalNames[i][1] = valFromSettings;
            }
        }
        emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
    }
}

//----------------------------------------------------------
QStringList ManagerEntryTables::namesBank() const
{
    return entryDisplayBanks().keys();
}
//----------------------------------------------------------
QStringList ManagerEntryTables::namesSale() const
{
    return entryDisplaySale().keys();
}
//----------------------------------------------------------
QString ManagerEntryTables::namePurchases() const
{
    return tr("Achats");
}
//----------------------------------------------------------
QMap<QString, EntryParserBankTable *> ManagerEntryTables::entryDisplayBanks() const
{
    return m_entryDisplayBanks;
}
//----------------------------------------------------------
QMap<QString, EntryParserOrdersTable *> ManagerEntryTables::entryDisplaySale() const
{
    return m_entryDisplaySales;
}
//----------------------------------------------------------
EntryParserPurchasesTable *ManagerEntryTables::entryDisplayPurchase() const
{
    return m_entryDisplayPurchase;
}
//----------------------------------------------------------
EntryParserImportationsTable *ManagerEntryTables::entryDisplayImportations() const
{
    return m_entryDisplayImportation;
}
//----------------------------------------------------------
const EntryParserAmazonPayments *ManagerEntryTables::entryParserAmazonPayments() const
{
    for (auto entryParserTable : m_entryDisplaySales) {
        const EntryParserAmazonPayments *entryParser
                = dynamic_cast<const EntryParserAmazonPayments *>(
                    entryParserTable->saleParser());
        if (entryParser != nullptr) {
            return entryParser;
        }
    }
    return nullptr;
}
//----------------------------------------------------------
QString ManagerEntryTables::journalName(const QString &entryTypeName) const
{
    return m_journalNamesHash.value(entryTypeName, "");
}
//----------------------------------------------------------
void ManagerEntryTables::load(int year)
{
    for (auto entryDisplay : m_entryDisplayBanks) {
        entryDisplay->load(year);
    }
    m_entryDisplayPurchase->load(year-1);
    m_entryDisplayPurchase->load(year);
    for (auto entryDisplay : m_entryDisplaySales) {
        entryDisplay->load(year-1);
        entryDisplay->load(year);
    }
    for (auto entryVarious : m_entryDisplayVarious) {
        entryVarious->load(year);
    }
}
//----------------------------------------------------------
AccountingEntries ManagerEntryTables::entries(int year) const
{
    auto allEntries = *m_entryDisplayPurchase->entries();
    for (auto entryTable : m_entryDisplayBanks) {
        addEntriesStatic(allEntries, *entryTable->entries());
    }
    for (auto entryTable : m_entryDisplaySales) {
        addEntriesStatic(allEntries, *entryTable->entries());
    }
    for (auto entryTable : m_entryDisplayVarious) {
        addEntriesStatic(allEntries, *entryTable->entries());
    }
    for (auto yearEntry : allEntries.keys()) {
        if (yearEntry != year) {
            allEntries.remove(yearEntry);
        }
    }
    return allEntries;
}
//----------------------------------------------------------
QVariant ManagerEntryTables::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QStringList values = {tr("Journal"), tr("Nom du journal")};
        return values[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int ManagerEntryTables::rowCount(const QModelIndex &) const
{
    return m_journalNames.size();
}
//----------------------------------------------------------
int ManagerEntryTables::columnCount(const QModelIndex &) const
{
    return 2;
}
//----------------------------------------------------------
QVariant ManagerEntryTables::data(
        const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_journalNames[index.row()][index.column()];
    }
    return QVariant();
}
//----------------------------------------------------------
Qt::ItemFlags ManagerEntryTables::flags(const QModelIndex &index) const
{
    if (index.column() == 1) {
        return Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }
    return Qt::ItemIsEnabled;
}
//----------------------------------------------------------
bool ManagerEntryTables::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        m_journalNames[index.row()][index.column()] = value.toString();
        m_journalNamesHash[m_journalNames[index.row()][0]] = m_journalNames[index.row()][1];
        saveFromSettings();
        return true;
    }
    return false;
}
//----------------------------------------------------------
