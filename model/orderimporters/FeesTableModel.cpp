#include <QtCore/qhash.h>
#include <QtCore/qsettings.h>
#include <QtCore/qsharedpointer.h>

#include "model/SettingManager.h"

#include "FeesTableModel.h"
#include "FeesAccountManager.h"
#include "model/CustomerManager.h"

//----------------------------------------------------------
FeesTableModel::FeesTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    QString selectedCustomerId = CustomerManager::instance()->getSelectedCustomerId();
    if (!selectedCustomerId.isEmpty()) {
        onCustomerSelectedChanged(selectedCustomerId);
    }
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            this,
            &FeesTableModel::onCustomerSelectedChanged);
}
//----------------------------------------------------------
FeesTableModel *FeesTableModel::instance(const QString &importerName)
{
    static QHash<QString, QSharedPointer<FeesTableModel>> instances;
    if (!instances.contains(importerName)) {
        instances[importerName] = QSharedPointer<FeesTableModel>(new FeesTableModel);
    }
    return instances[importerName].data();
}
//----------------------------------------------------------
void FeesTableModel::addFees(const QString &importerName, const QString &feeTitle)
{
    instance(importerName)->addFees(feeTitle);
}
//----------------------------------------------------------
void FeesTableModel::addFees(const QString &feeTitle)
{
    int lenBefore = m_allFeeNames.size();
    m_allFeeNames << feeTitle;
    if (lenBefore < m_allFeeNames.size()) {
        QStringList elements;
        elements << feeTitle;
        elements << "-";
        beginInsertRows(QModelIndex(), 0, 0);
        m_fees.insert(0, elements);
        endInsertRows();
    }
}
//----------------------------------------------------------
void FeesTableModel::saveInSettings() const
{
    QStringList elements;
    for (auto fee : m_fees) {
        elements << fee.join(":::");
    }
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (elements.size() > 0) {
        QString stringCompressed = elements.join(";;;");
        settings.setValue(m_settingKey, stringCompressed);
    } else if (settings.contains(m_settingKey)) {
        settings.remove(m_settingKey);
    }
}
//----------------------------------------------------------
void FeesTableModel::loadFromSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (m_fees.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_fees.size()-1);
        m_fees.clear();
        m_allFeeNames.clear();
        endRemoveRows();
    }
    if (settings.contains(m_settingKey)) {
        QString stringCompressed = settings.value(m_settingKey).toString();
        QStringList elements = stringCompressed.split(";;;");
        for (auto element : elements) {
            QStringList values = element.split(":::");
            m_fees << values;
            m_allFeeNames << values[0];
        }
        beginInsertRows(QModelIndex(), 0, m_fees.size()-1);
        endInsertRows();
    }
}
//----------------------------------------------------------
void FeesTableModel::sortByAccount()
{
    std::sort(m_fees.begin(), m_fees.end(),
              [](const QStringList &left, const QStringList &right) -> bool {
        return left[0] < right[0];
    });
    emit dataChanged(this->index(0, 0),
                     this->index(m_fees.size()-1, columnCount()-1));
}
//----------------------------------------------------------
QVariant FeesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant value;
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QStringList values = {"Titre", "Compte"};
        value = values[section];
    }
    return value;
}
//----------------------------------------------------------
int FeesTableModel::rowCount(const QModelIndex &) const
{
    return m_fees.size();
}
//----------------------------------------------------------
int FeesTableModel::columnCount(const QModelIndex &) const
{
    return 2;
}
//----------------------------------------------------------
QVariant FeesTableModel::data(const QModelIndex &index, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole) {
        value = m_fees[index.row()][index.column()];
    }
    return value;
}
//----------------------------------------------------------
void FeesTableModel::onCustomerSelectedChanged(const QString &customerId)
{
    if (customerId.isEmpty()) {
        m_settingKey = "";
        beginRemoveRows(QModelIndex(), 0, m_fees.size()-1);
        m_fees.clear();
        m_allFeeNames.clear();
        endRemoveRows();
    } else {
        m_settingKey = "FeesTableModel-" + customerId;
        loadFromSettings();
    }
}
//----------------------------------------------------------
