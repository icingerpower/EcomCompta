#include <QtCore/qsettings.h>
#include <QtGui/qbrush.h>
#include <QtGui/qcolor.h>

#include "ModelStockDeported.h"
#include "model/CustomerManager.h"
#include "model/SettingManager.h"
#include "model/inventory/InventoryManager.h"
#include "model/orderimporters/ImporterYearsManager.h"

//----------------------------------------------------------
ModelStockDeported::ModelStockDeported(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_rate = 1.;
    m_defaultValue = 0.;
    QString selectedCustomerId = CustomerManager::instance()->getSelectedCustomerId();
    if (!selectedCustomerId.isEmpty()) {
        onCustomerSelectedChanged(selectedCustomerId);
    }
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            this,
            &ModelStockDeported::onCustomerSelectedChanged);
}
//----------------------------------------------------------
void ModelStockDeported::onCustomerSelectedChanged(
        const QString &customerId)
{
    if (customerId.isEmpty()) {
        m_settingKey = "";
        _clear();
    } else {
        m_settingKey = "ModelStockDeported-" + customerId;
        loadFromSettings();
    }
}
//----------------------------------------------------------
double ModelStockDeported::defaultValue() const
{
    return m_defaultValue;
}
//----------------------------------------------------------
void ModelStockDeported::setDefaultValue(double defaultValue)
{
    m_defaultValue = defaultValue;
}
//----------------------------------------------------------
double ModelStockDeported::percentage() const
{
    return m_rate * 100;
}
//----------------------------------------------------------
void ModelStockDeported::setPercentage(double percentage)
{
    m_rate = percentage / 100.;
    saveInSettings();
}
//----------------------------------------------------------
double ModelStockDeported::rate() const
{
    return m_rate;
}
//----------------------------------------------------------
void ModelStockDeported::setRate(double rate)
{
    m_rate = rate;
    saveInSettings();
}
/*
//----------------------------------------------------------
QString ModelStockDeported::m_settingKeyChanged() const
{
    return m_settingKey + "-Changed";
}
//----------------------------------------------------------
QString ModelStockDeported::m_settingKeyOrigValues() const
{
    return m_settingKey + "-orig-values";
}
//*/
//----------------------------------------------------------
QString ModelStockDeported::m_settingKeyComputingChoice() const
{
    return m_settingKey + "-computing-choice";
}
//----------------------------------------------------------
QString ModelStockDeported::m_settingKeyPercentage() const
{
    return m_settingKey + "-percentage";
}
//----------------------------------------------------------
StockDeportedComputing ModelStockDeported::computingType() const
{
    return m_computingChoice;
}
//----------------------------------------------------------
void ModelStockDeported::setComputingType(const StockDeportedComputing &computingType)
{
    if (computingType != m_computingChoice) {
        m_computingChoice = computingType;
        saveInSettings();
    }
}
//----------------------------------------------------------
void ModelStockDeported::_clear()
{
    if (m_skusChanged.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, rowCount()-1);
        m_skusChanged.clear();
        m_skus.clear();
        m_skuValues.clear();
        endRemoveRows();
    }
}
//----------------------------------------------------------
ModelStockDeported *ModelStockDeported::instance()
{
    static ModelStockDeported instance;
    return &instance;
}
//----------------------------------------------------------
void ModelStockDeported::recordSku(const QString &sku, const QString &title, double value)
{
    if (!m_skuValues.contains(sku)) {
        beginInsertRows(QModelIndex(), 0, 0);
        QString valueString = QString::number(value, 'f', 2);
        m_skus.insert(0, sku);
        m_skuValues[sku] = QStringList({sku, title, valueString, valueString});
        //m_skusOrigValues[sku] = value;
        endInsertRows();
        saveInSettings();
    }
}
//----------------------------------------------------------
void ModelStockDeported::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!m_settingKey.isEmpty()) {
        if (!m_skuValues.isEmpty()) {
            QStringList elements;
            for (auto sku : m_skus) {
                QString element = m_skuValues[sku].join(":::");
                elements << element;
            }
            settings.setValue(m_settingKey, elements.join(";;;"));
        } else if (settings.contains(m_settingKey)) {
            settings.remove(m_settingKey);
        }
        QString settingKeyComputing = m_settingKeyComputingChoice();
        settings.setValue(settingKeyComputing, m_computingChoice);
        QString settingKeyPercentage = m_settingKeyPercentage();
        settings.setValue(settingKeyPercentage, m_rate);
    }
}
//----------------------------------------------------------
void ModelStockDeported::loadFromSettings()
{
    _clear();
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!m_settingKey.isEmpty()) {
        if (settings.contains(m_settingKey)) {
            QString linesString = settings.value(m_settingKey).toString();
            QStringList lines = linesString.split(";;;");
            for (auto line : lines) {
                QStringList elements = line.split(":::");
                m_skus << elements[0];
                m_skuValues[elements[0]] = elements;
                if (elements[2] != elements[3]) {
                    m_skusChanged << elements[0];
                }
            }
        }
        QString settingKeyPercentage = m_settingKeyPercentage();
        if (settings.contains(settingKeyPercentage)) {
            m_rate = settings.value(settingKeyPercentage).toDouble();
        }
        /*
        QString settingKeyChanged = m_settingKeyChanged();
        if (settings.contains(settingKeyChanged)) {
            m_skusChanged = settings.value(settingKeyChanged).toString().split(";;;").toSet();
        }
        //*/
        if (m_skuValues.size() > 0) {
            beginInsertRows(QModelIndex(), 0, m_skuValues.size()-1);
            endInsertRows();
        }
        QString settingKeyComputing = m_settingKeyComputingChoice();
        if (settings.contains(settingKeyComputing)) {
            m_computingChoice = static_cast<StockDeportedComputing>(settings.value(settingKeyComputing).toInt());
        }
    }
}
//----------------------------------------------------------
void ModelStockDeported::loadFromInventoryManager()
{
    QSet<QString> skusNotDone = m_skus.toSet();
    QSet<QString> skusDone;
    auto allYears = ImporterYearsManager::instance()->years();
    std::sort(allYears.begin(), allYears.end());
    for (auto itYear = allYears.rbegin(); itYear != allYears.rend(); ++itYear) {
        InventoryManager::instance()->load(*itYear);
        skusNotDone.subtract(skusDone);
        for (auto sku : skusNotDone) {
            double value = InventoryManager::instance()
                    ->valueUnitAverage(sku);
            if (qAbs(value) > 0.001) {
                skusDone << sku;
                if (!m_skusChanged.contains(sku)) {
                    m_skusChanged << sku;
                }
                m_skuValues[sku][2] = QString::number(value, 'f', 2);
            }
        }
    }
    emit dataChanged(index(0, 2), index(m_skuValues.size()-1, 2));
}
//----------------------------------------------------------
void ModelStockDeported::resetValue(const QModelIndex &index)
{
    QString sku = m_skus[index.row()];
    //if (m_skuValues[sku][2] != m_skuValues[sku][3]) {
    if (m_skusChanged.contains(sku)) {
        m_skuValues[sku][2] = m_skuValues[sku][3];
        m_skusChanged.remove(sku);
        saveInSettings();
        auto indexLeft = this->index(index.row(), 0, QModelIndex());
        auto indexRight = this->index(index.row(), 2, QModelIndex());
        emit dataChanged(indexLeft, indexRight);
    }
    //m_skuValues[index.row()][2]
    //= QString::number(m_skusOrigValues.take(m_skuValues[index.row()][0]), 'f', 2);
}
//----------------------------------------------------------
double ModelStockDeported::inventoryValue(const QString &sku)
{
    if (!m_skuValues.contains(sku)) {
        recordSku(sku, sku, 0.);
        return m_defaultValue;
    }
    if (m_computingChoice == StockDeportedComputing::DontCompute) {
        return 0.;
    } else if (m_computingChoice == StockDeportedComputing::Percentage){
        return m_skuValues[sku][3].toDouble() * m_rate;
    } else {
        return m_skuValues[sku][2].toDouble();
    }
}
//----------------------------------------------------------
QVariant ModelStockDeported::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QStringList values = {tr("SKU"), tr("Nom"), tr("Valeur")};
        return values[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int ModelStockDeported::rowCount(const QModelIndex &) const
{
    return m_skus.size();
}
//----------------------------------------------------------
int ModelStockDeported::columnCount(const QModelIndex &) const
{
    return 3;
}
//----------------------------------------------------------
Qt::ItemFlags ModelStockDeported::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled
            | Qt::ItemIsSelectable;
    if (index.column() > 0) {
        flags |= Qt::ItemIsEditable;
    }
    return flags;
}
//----------------------------------------------------------
QVariant ModelStockDeported::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_skuValues[m_skus[index.row()]][index.column()];
    } else if (role == Qt::BackgroundRole) {
        if (m_skusChanged.contains(m_skus[index.row()])) {
            static QVariant variantOrange = SettingManager::instance()->brushOrange();
            return variantOrange;
        } else if (m_skuValues[m_skus[index.row()]][2].toDouble() < 0.001) {
            static QVariant variantRed = SettingManager::instance()->brushRed();
            return variantRed;
        } else {
            static QVariant variantGreen = SettingManager::instance()->brushGreen();
            return variantGreen;
        }
    }
    return QVariant();
}
//----------------------------------------------------------
bool ModelStockDeported::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        QString sku = m_skus[index.row()];
        if (!m_skusChanged.contains(sku)) {
            m_skusChanged << sku;
        }
        m_skuValues[m_skus[index.row()]][index.column()] = value.toString();
        saveInSettings();
        return true;
    }
    return false;
}
//----------------------------------------------------------

