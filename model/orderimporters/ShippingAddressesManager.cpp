#include <QtCore/qsettings.h>

#include "model/SettingManager.h"
#include "model/CustomerManager.h"

#include "ShippingAddressesManager.h"

//----------------------------------------------------------
ShippingAddressesManager::ShippingAddressesManager(QObject *parent)
    : QAbstractListModel(parent)
{
    init(QString());
}
//----------------------------------------------------------
void ShippingAddressesManager::init(const QString &settingPrefix)
{
    QString selectedCustomerId = CustomerManager::instance()->getSelectedCustomerId();
    if (!selectedCustomerId.isEmpty()) {
        onCustomerSelectedChangedForInit(
                    selectedCustomerId, settingPrefix);
    }
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            this,
            &ShippingAddressesManager::onCustomerSelectedChanged);
}
//----------------------------------------------------------
ShippingAddressesManager *ShippingAddressesManager::instance()
{
    static ShippingAddressesManager instance;
    return &instance;
}
//----------------------------------------------------------
Address ShippingAddressesManager::getAddressCompany() const
{
    return m_addresses[0];
}
//----------------------------------------------------------
QString ShippingAddressesManager::companyCountryName() const
{
    return m_addresses[0].countryName();
}
//----------------------------------------------------------
QString ShippingAddressesManager::companyCountryCode() const
{
    return m_addresses[0].countryCode();
}
//----------------------------------------------------------
Address ShippingAddressesManager::getAddress(
        const QString &internalId) const
{
    if (m_addressesById.contains(internalId)) {
        return m_addressesById[internalId];
    }
    return Address();
}
//----------------------------------------------------------
Address ShippingAddressesManager::getAddress(int index) const
{
    return m_addresses[index];
}
//----------------------------------------------------------
bool ShippingAddressesManager::contains(const QString &internalId) const
{
    return m_addressesById.contains(internalId);
}
//----------------------------------------------------------
void ShippingAddressesManager::saveAddressesInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (m_addresses.size() > 0) {
        QStringList addString;
        for (auto address : m_addresses) {
            addString << address.toString();
        }
        QString compressedString = addString.join(",,,");
        settings.setValue(m_settingKey, compressedString);
    } else if (settings.contains(m_settingKey)) {
        settings.remove(m_settingKey);
    }
}
//----------------------------------------------------------
void ShippingAddressesManager::loadAddressesInSettings()
{
    if (m_addresses.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_addresses.size()-1);
        m_addresses.clear();
        m_addressesById.clear();
        endRemoveRows();
    }
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(m_settingKey)) {
        QString compressedString = settings.value(m_settingKey).toString();
        auto elements = compressedString.split(",,,");
        for (auto element : elements) {
            Address address = Address::fromString(element);
            m_addresses << address;
            m_addressesById[address.internalId()] = address;
        }
    }
    beginInsertRows(QModelIndex(), 0, m_addresses.size()-1);
    endInsertRows();
}
//----------------------------------------------------------
void ShippingAddressesManager::addAddress(const Address &address)
{
    beginInsertRows(QModelIndex(), m_addresses.size(), m_addresses.size());
    m_addresses << address;
    m_addressesById[address.internalId()] = address;
    saveAddressesInSettings();
    endInsertRows();
}
//----------------------------------------------------------
void ShippingAddressesManager::updateAddress(int index, const Address &address)
{
    m_addressesById.remove(m_addresses[index].internalId());
    m_addresses[index].copyFromOther(address);
    m_addressesById[address.internalId()] = address;
    saveAddressesInSettings();
}
//----------------------------------------------------------
void ShippingAddressesManager::removeAddress(int index)
{
    if (index > 0) { /// The first address of the company can't be deleted
        QString internalId = m_addresses[index].internalId();
        emit adressAboutToBeRemoved(internalId);
        beginRemoveRows(QModelIndex(), index, index);
        m_addresses.removeAt(index);
        m_addressesById.remove(internalId);
        saveAddressesInSettings();
        endRemoveRows();
        emit adressRemoved(internalId);
    }
}
//----------------------------------------------------------
int ShippingAddressesManager::rowCount(const QModelIndex &) const
{
    return m_addresses.size();
}
//----------------------------------------------------------
int ShippingAddressesManager::columnCount(const QModelIndex &) const
{
    return 1;
}
//----------------------------------------------------------
QVariant ShippingAddressesManager::data(
        const QModelIndex &index, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole) {
        value = m_addresses[index.row()].label();
    }
    return value;
}
//----------------------------------------------------------
void ShippingAddressesManager::onCustomerSelectedChanged(
        const QString &customerId)
{
    onCustomerSelectedChangedForInit(customerId, settingKeyPrefix());
}
//----------------------------------------------------------
void ShippingAddressesManager::onCustomerSelectedChangedForInit(
        const QString &customerId,
        const QString &settingPrefix)
{
    if (customerId.isEmpty()) {
        m_settingKey = settingKeyPrefix();
        beginRemoveRows(QModelIndex(), 0, m_addresses.size()-1);
        m_addresses.clear();
        m_addressesById.clear();
        endRemoveRows();
    } else {
        m_settingKey = "ShippingAddresses-" + settingPrefix + customerId;
        loadAddressesInSettings();
    }

}
//----------------------------------------------------------
QString ShippingAddressesManager::settingKeyPrefix() const
{
    return QString();
}
//----------------------------------------------------------

