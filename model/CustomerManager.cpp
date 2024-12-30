#include <QtCore/qsettings.h>

#include "SettingManager.h"

#include "CustomerManager.h"

//----------------------------------------------------------
CustomerManager *CustomerManager::instance()
{
    static CustomerManager instance;
    return &instance;
}
//----------------------------------------------------------
CustomerManager::CustomerManager(QObject *object)
    : QAbstractListModel(object)
{
    loadCustomers();
}
//----------------------------------------------------------
void CustomerManager::selectCustomer(
        const QString &customerName)
{
    m_selectedCustomer = customerName;
    QString id;
    if (!customerName.isEmpty()) {
        id = m_customers[m_selectedCustomer].internalId();
    }
    emit selectedCustomerChanged(id);
}
//----------------------------------------------------------
QString CustomerManager::getSelectedCustomerId() const
{
    return m_customers[m_selectedCustomer].internalId();
}
//----------------------------------------------------------
QString CustomerManager::getSelectedCustomerCurrency() const
{
    return m_customers[m_selectedCustomer].currency();
}
//----------------------------------------------------------
QString CustomerManager::getSelectedCustomerCountryCode() const
{
    return "FR"; //TODO
}
//----------------------------------------------------------
QString CustomerManager::getSelectedCustomer() const
{
    return m_selectedCustomer;
}
//----------------------------------------------------------
bool CustomerManager::isCustomerAvailable(
        const QString &name) const
{
    bool is = !m_customers.contains(name) && !name.isEmpty();
    return is;
}
//----------------------------------------------------------
void CustomerManager::addCustomerAvailable(
        const Customer &customer)
{
    beginInsertRows(QModelIndex(), m_customers.size(), m_customers.size());
    m_customers[customer.name()] = customer;
    m_customerNames << customer.name();
    saveCustomers();
    endInsertRows();
}
//----------------------------------------------------------
void CustomerManager::editCustomer(
        const QString &currentName,
        const Customer &customer)
{
    m_customers[customer.name()].copyFromOther(customer);
    if (currentName != customer.name()) {
        int indexCurrent = m_customerNames.indexOf(currentName);
        m_customerNames[indexCurrent] = customer.name();
        m_customers.remove(currentName);
        saveCustomers();
        emit dataChanged(index(indexCurrent), index(indexCurrent));
    } else {
        saveCustomers();
    }
}
//----------------------------------------------------------
void CustomerManager::removeCustomer(const QString &name)
{
    int index = m_customerNames.indexOf(name);
    removeCustomer(index);
}
//----------------------------------------------------------
void CustomerManager::removeCustomer(int index)
{
    QString name = m_customerNames[index];
    beginRemoveRows(QModelIndex(), index, index);
    m_customerNames.removeAt(index);
    m_customers.remove(name);
    if (name == m_selectedCustomer) {
        if (m_customers.size() > 0) {
            m_selectedCustomer = m_customers[0].name();
        } else {
            m_selectedCustomer = "";
        }
        emit selectedCustomerChanged(m_selectedCustomer);
    }
    saveCustomers();
    endRemoveRows();
}
//----------------------------------------------------------
QHash<QString, Customer> CustomerManager::getCustomers()
{
    return m_customers;
}
//----------------------------------------------------------
Customer CustomerManager::getCustomer(int index)
{
    return m_customers[m_customerNames[index]];
}
//----------------------------------------------------------
Customer CustomerManager::getCustomer(const QString &name)
{
    return m_customers[name];
}
//----------------------------------------------------------
QStringList CustomerManager::getCustomerNames()
{
    return m_customerNames;
}
//----------------------------------------------------------
int CustomerManager::nCustomers() const
{
    return  m_customers.size();
}
//----------------------------------------------------------
int CustomerManager::rowCount(const QModelIndex &) const
{
    return m_customers.size();
}
//----------------------------------------------------------
QVariant CustomerManager::data(const QModelIndex &index, int role) const
{
    QVariant variant;
    if (role == Qt::DisplayRole) {
        variant = m_customerNames[index.row()];
    }
    return variant;
}
//----------------------------------------------------------
void CustomerManager::saveCustomers()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (m_customers.size() > 0) {
        QStringList customersString;
        for (auto name : m_customerNames) {
            customersString << m_customers[name].toString();
        }
        QString compressedString = customersString.join(",,,");
        settings.setValue(SET_CUSTOMERS, compressedString);
    } else if (settings.contains(SET_CUSTOMERS)) {
        settings.remove(SET_CUSTOMERS);
    }
    if (!m_selectedCustomer.isEmpty()) {
        settings.setValue(SET_CUSTOMER_SELECTED, m_selectedCustomer);
    } else if (settings.contains(SET_CUSTOMER_SELECTED)) {
        settings.remove(SET_CUSTOMER_SELECTED);
    }
}
//----------------------------------------------------------
void CustomerManager::loadCustomers()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(SET_CUSTOMERS)) {
        QString compressedValue = settings.value(SET_CUSTOMERS).toString();
        if (!compressedValue.isEmpty()) {
            QStringList customersString = compressedValue.split(",,,");
            for (auto customerString : customersString) {
                Customer customer(customerString);
                m_customers[customer.name()] = customer;
                m_customerNames << customer.name();
            }
            beginInsertRows(QModelIndex(), 0, m_customers.size()-1);
            endInsertRows();
            if (settings.contains(SET_CUSTOMERS)) {
                m_selectedCustomer = settings.value(SET_CUSTOMER_SELECTED).toString();
            }
        }
        if (m_customers.size() > 0 && m_selectedCustomer.isEmpty()) {
            m_selectedCustomer = m_customers.keys()[0]; //TODO optimize as very slow
        }
    }
}
//----------------------------------------------------------
