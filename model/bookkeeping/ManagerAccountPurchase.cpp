#include <QSettings>

#include "../common/countries/CountryManager.h"

#include "ManagerAccountPurchase.h"
#include "model/SettingManager.h"

//----------------------------------------------------------
ManagerAccountPurchase *ManagerAccountPurchase::instance()
{
    static ManagerAccountPurchase instance;
    return &instance;
}
//----------------------------------------------------------
void ManagerAccountPurchase::onCustomerSelectedChanged(const QString &customerId)
{
    if (customerId.isEmpty()) {
        _clear();
    } else {
        loadFromSettings();
    }
}
//----------------------------------------------------------
QString ManagerAccountPurchase::uniqueId() const
{
    return "ManagerAccountPurchase";
}
//----------------------------------------------------------
ManagerAccountPurchase::ManagerAccountPurchase(QObject *parent)
    : QAbstractTableModel(parent), UpdateToCustomer()
{
    QString country = QObject::tr(
                "France",
                "The company country. For instance Spain if the application is translate in spain for spanish market");
    // TODO more countries + create accounts
    m_accounts << QStringList({country, "445660"});
    m_accounts << QStringList({CountryManager::GERMANY, "445630"});
    for (auto account : qAsConst(m_accounts)) {
        m_accountsByCountry[account[0]] = account[1];
    }
    init();
    QString key = settingKey();
    int TEMP=10;++TEMP;
}
//----------------------------------------------------------
void ManagerAccountPurchase::_clear()
{
    beginRemoveRows(QModelIndex(), 0, rowCount()-1);
    m_accounts.clear();
    m_accountsByCountry.clear();
    endRemoveRows();
}
//----------------------------------------------------------
QString ManagerAccountPurchase::accountVat(const QString &countryName) const
{
    return m_accountsByCountry.value(countryName, "");
}
//----------------------------------------------------------
void ManagerAccountPurchase::addAccount(
        const QString &country, const QString &account)
{
    beginInsertRows(QModelIndex(), 0, 0);
    m_accounts << QStringList({country, account});
    m_accountsByCountry[country] = account;
    saveInSettings();
    endInsertRows();
}
//----------------------------------------------------------
void ManagerAccountPurchase::removeAccount(const QModelIndex &index)
{
    beginRemoveRows(QModelIndex(), index.row(), index.row());
    QString country = m_accounts[index.row()][0];
    m_accountsByCountry.remove(country);
    m_accounts.removeAt(index.row());
    saveInSettings();
    endRemoveRows();
}
//----------------------------------------------------------
void ManagerAccountPurchase::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!settingKey().isEmpty()) {
        QStringList lines;
        for (auto it=m_accounts.begin();
             it!=m_accounts.end();
             ++it) {
            QStringList elements
                    = {it->value(0), it->value(1)};
            lines << elements.join(";;;");
        }
        settings.setValue(settingKey(), lines.join(":::"));
    }
}
//----------------------------------------------------------
void ManagerAccountPurchase::loadFromSettings()
{
    //return; //TODO
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!settingKey().isEmpty()) {
        if (settings.contains(settingKey())) {
            _clear();
            QString text = settings.value(settingKey()).toString();
            QStringList lines = text.split(":::");
            for (auto line:lines) {
                QStringList elements = line.split(";;;");
                QString country = elements.takeFirst();
                QString account = elements.takeFirst();
                m_accounts << QStringList({country, account});
                m_accountsByCountry[country] = account;
            }
        }
    }
    if (m_accounts.size() > 0) {
        beginInsertRows(QModelIndex(), 0, m_accounts.size()-1);
        endInsertRows();
    }
}
//----------------------------------------------------------
QVariant ManagerAccountPurchase::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QStringList values = {tr("Pays"), tr("Compte TVA achat")};
        return values[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int ManagerAccountPurchase::rowCount(const QModelIndex &) const
{
    return m_accounts.size();
}
//----------------------------------------------------------
int ManagerAccountPurchase::columnCount(const QModelIndex &) const
{
    return 2;
}
//----------------------------------------------------------
QVariant ManagerAccountPurchase::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_accounts[index.row()][index.column()];
    }
    return QVariant();
}
//----------------------------------------------------------
Qt::ItemFlags ManagerAccountPurchase::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
}
//----------------------------------------------------------
bool ManagerAccountPurchase::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        if (m_accounts[index.row()][index.column()] != value.toString()) {
            m_accounts[index.row()][index.column()] = value.toString();
            saveInSettings();
            return true;
        }
    }
    return false;
}
//----------------------------------------------------------

