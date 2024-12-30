#include <QSettings>

#include "TableBankBeginBalances.h"

#include "model/bookkeeping/entries/parsers/ManagerEntryTables.h"
#include "model/bookkeeping/entries/parsers/EntryParserBankTable.h"
#include "model/SettingManager.h"

//----------------------------------------------------------
TableBankBeginBalances::TableBankBeginBalances(QObject *parent)
    : QAbstractTableModel(parent)
{
    const auto &banks = ManagerEntryTables::instance()->entryDisplayBanks();
    int index = 0;
    for (const auto &bank : banks) {
        const QString &name = bank->name();
        m_bankNames << name;
        m_bankNamesToIndex[name] = index;
        m_amounts << 0.;
        ++index;
    }
}
//----------------------------------------------------------
TableBankBeginBalances *TableBankBeginBalances::instance()
{
    static TableBankBeginBalances instance;
    static bool initialized = false;
    if (!initialized) {
        instance.loadFromSettings();
        initialized = true;
    }
    return &instance;
}
//----------------------------------------------------------
void TableBankBeginBalances::onCustomerSelectedChanged(
        const QString &customerId)
{
    if (customerId.isEmpty()) {
        _clear();
    } else {
        loadFromSettings();
    }
}
//----------------------------------------------------------
QString TableBankBeginBalances::uniqueId() const
{
    return "TableBankBeginBalances";
}
//----------------------------------------------------------
void TableBankBeginBalances::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    settings.setValue(settingsKeyBankNames(), m_bankNames);
    QStringList amounts;
    for (auto amount : m_amounts) {
        amounts << QString::number(amount, 'f', 2);
    }
    settings.setValue(settingsKeyBalances(), amounts);
}
//----------------------------------------------------------
void TableBankBeginBalances::loadFromSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    const QStringList &bankNames = settings.value(settingsKeyBankNames()).toStringList();
    const QStringList &balances = settings.value(settingsKeyBalances()).toStringList();
    QHash<QString, double> nameToAmount;
    for (int i=0; i<bankNames.size(); ++i) {
        nameToAmount[bankNames[i]] = balances[i].toDouble();
    }
    for (int i=0; i<m_bankNames.size(); ++i) {
        if (nameToAmount.contains(m_bankNames[i])) {
            m_amounts[i] = nameToAmount[m_bankNames[i]];
        }
    }
}
//----------------------------------------------------------
void TableBankBeginBalances::_clear()
{
    beginRemoveRows(QModelIndex(), 0, rowCount()-1);
    for (auto &balance : m_amounts) {
        balance = 0;
    }
    endRemoveRows();
}
//----------------------------------------------------------
QString TableBankBeginBalances::settingsKeyBankNames() const
{
    return settingKey() + "-bankNames";
}
//----------------------------------------------------------
QString TableBankBeginBalances::settingsKeyBalances() const
{
    return settingKey() + "-balances";
}
//----------------------------------------------------------
double TableBankBeginBalances::getBeginAmount(const QString &bankName) const
{
    return m_amounts[m_bankNamesToIndex[bankName]];
}
//----------------------------------------------------------
QVariant TableBankBeginBalances::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return m_bankNames[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int TableBankBeginBalances::rowCount(const QModelIndex &) const
{
    return 1;
}
//----------------------------------------------------------
int TableBankBeginBalances::columnCount(const QModelIndex &) const
{
    static const int count = ManagerEntryTables::instance()->entryDisplayBanks().size();
    return count;
}
//----------------------------------------------------------
QVariant TableBankBeginBalances::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_amounts[index.column()];
    }
    return QVariant();
}
//----------------------------------------------------------
bool TableBankBeginBalances::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        m_amounts[index.column()] = value.toDouble();
        saveInSettings();
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}
//----------------------------------------------------------
Qt::ItemFlags TableBankBeginBalances::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}
