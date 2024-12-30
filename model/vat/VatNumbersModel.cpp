#include <QtCore/qsettings.h>

#include "model/SettingManager.h"

#include "VatNumbersModel.h"
#include "model/CustomerManager.h"

//----------------------------------------------------------
VatNumbersModel::VatNumbersModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_iossThreshold = false;
    QString selectedCustomerId = CustomerManager::instance()->getSelectedCustomerId();
    if (!selectedCustomerId.isEmpty()) {
        onCustomerSelectedChanged(selectedCustomerId);
    }
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            this,
            &VatNumbersModel::onCustomerSelectedChanged);
}
//----------------------------------------------------------
QString VatNumbersModel::m_settingKeyIoss() const
{
    return m_settingKey + "-IOSS";
}
//----------------------------------------------------------
QString VatNumbersModel::m_settingKeyIossThreshold() const
{
    return m_settingKey + "-IOSS-THRESHOLD";
}
//----------------------------------------------------------
bool VatNumbersModel::hasIossThreshold() const
{
    return m_iossThreshold;
}
//----------------------------------------------------------
void VatNumbersModel::setIossThreshold(bool iossThreshold)
{
    if (m_iossThreshold != iossThreshold) {
        m_iossThreshold = iossThreshold;
        emit iossThresholdChanded(m_iossThreshold);
    }
    saveInSettings();
}
//----------------------------------------------------------
VatNumberData VatNumbersModel::iossNumber() const
{
    return m_iossNumber;
}
//----------------------------------------------------------
void VatNumbersModel::setIossNumber(const VatNumberData &iossNumber)
{
    if (m_iossNumber != iossNumber) {
        m_iossNumber = iossNumber;
        emit iossNumberChanged(m_iossNumber);
    }
    saveInSettings();
}
//----------------------------------------------------------
double VatNumbersModel::iossThreshold() const
{
    return m_iossThreshold ? 10000. : 0.;
}
//----------------------------------------------------------
VatNumbersModel *VatNumbersModel::instance()
{
    static VatNumbersModel instance;
    return &instance;
}
//----------------------------------------------------------
void VatNumbersModel::clear()
{
    if (m_vatNumbers.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_vatNumbers.size()-1);
        m_vatNumbers.clear();
        endRemoveRows();
    }
}
//----------------------------------------------------------
void VatNumbersModel::onCustomerSelectedChanged(const QString &customerId)
{
    if (customerId.isEmpty()) {
        m_settingKey = "";
        clear();
    } else {
        m_settingKey = "VatNumbersModel-" + customerId;
        loadFromSettings();
    }
}
//----------------------------------------------------------
void VatNumbersModel::loadFromSettings()
{
    clear();
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(m_settingKey)) {
        QString string = settings.value(m_settingKey).toString();
        QStringList elements = string.split(",,,");
        for (auto element : elements) {
            VatNumberData data = VatNumberData::fromString(element);
            m_vatNumberIndexesByCountry[data.number.left(2)] = m_vatNumbers.size();
            m_vatNumbers << data;
        }
    }
    QString settingKeyIoss = m_settingKeyIoss();
    if (settings.contains(settingKeyIoss)) {
        auto before = m_iossNumber;
        m_iossNumber = VatNumberData::fromString(
                    settings.value(settingKeyIoss).toString());
        if (before != m_iossNumber) {
            emit iossNumberChanged(m_iossNumber);
        }
    }
    QString settingKeyIossThreshold = m_settingKeyIossThreshold();
    if (settings.contains(settingKeyIossThreshold)) {
        auto before = m_iossThreshold;
        m_iossThreshold = settings.value(settingKeyIossThreshold).toBool();
        if (before != m_iossThreshold) {
            emit iossThresholdChanded(m_iossThreshold);
        }
    }
    beginInsertRows(QModelIndex(), 0, m_vatNumbers.size()-1);
    endInsertRows();
}
//----------------------------------------------------------
void VatNumbersModel::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (m_vatNumbers.size() > 0) {
        QStringList elements;
        for (auto vatNumber : m_vatNumbers) {
            elements << vatNumber.toString();
        }
        settings.setValue(m_settingKey, elements.join(",,,"));
    } else if (!m_settingKey.isEmpty() && settings.contains(m_settingKey)) {
        settings.remove(m_settingKey);
    }
    QString settingKeyIoss = m_settingKeyIoss();
    if (!m_iossNumber.number.isEmpty()) {
        settings.setValue(settingKeyIoss, m_iossNumber.toString());
    } else {
        settings.remove(settingKeyIoss);
    }
    settings.setValue(m_settingKeyIossThreshold(), m_iossThreshold);
}
//----------------------------------------------------------
void VatNumbersModel::add(const QString &number, const QDate &date)
{
    VatNumberData vatNumber = {number, date};
    add(vatNumber);
}
//----------------------------------------------------------
void VatNumbersModel::add(const VatNumberData &vatNumber)
{
    beginInsertRows(QModelIndex(), m_vatNumbers.size(), m_vatNumbers.size());
    m_vatNumberIndexesByCountry[vatNumber.number.left(2)] = m_vatNumbers.size();
    m_vatNumbers << vatNumber;
    saveInSettings();
    endInsertRows();
}
//----------------------------------------------------------
void VatNumbersModel::remove(const QModelIndex &index)
{
    beginRemoveRows(QModelIndex(), index.row(), index.row());
    m_vatNumberIndexesByCountry.remove(m_vatNumbers[index.row()].countryCode());
    m_vatNumbers.removeAt(index.row());
    saveInSettings();
    endRemoveRows();
}
//----------------------------------------------------------
bool VatNumbersModel::containsCountry(const QString &countryCode)
{
    return m_vatNumberIndexesByCountry.contains(countryCode);
}
//----------------------------------------------------------
QVariant VatNumbersModel::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    QVariant value;
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QStringList values = {tr("Numéro"), tr("Date d'enregistrement")};
        return values[section];
    }
    return value;
}
//----------------------------------------------------------
int VatNumbersModel::rowCount(const QModelIndex &) const
{
    return m_vatNumbers.size();
}
//----------------------------------------------------------
int VatNumbersModel::columnCount(const QModelIndex &) const
{
    return 2;
}
//----------------------------------------------------------
QVariant VatNumbersModel::data(const QModelIndex &index, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        value = m_vatNumbers[index.row()].value(index.column());
    }
    return value;
}
//----------------------------------------------------------
bool VatNumbersModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool changed = false;
    if (data(index, role) != value && role == Qt::EditRole) {
        QString beforeCountryCode = m_vatNumbers[index.row()].countryCode();
        m_vatNumbers[index.row()].setValue(index.column(), value);
        if (m_vatNumbers[index.row()].countryCode() != beforeCountryCode) {
            m_vatNumberIndexesByCountry.remove(beforeCountryCode);
            m_vatNumberIndexesByCountry[m_vatNumbers[index.row()].countryCode()] = index.row();
        }
        changed = true;
        saveInSettings();
    }
    return changed;
}
//----------------------------------------------------------
Qt::ItemFlags VatNumbersModel::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}
//----------------------------------------------------------
QVariant VatNumberData::value(int index) const
{
    static QList<std::function<QVariant(const VatNumberData *)>> functionValues = {
            [](const VatNumberData *instance)->QVariant{ return instance->number;},
            [](const VatNumberData *instance)->QVariant{ return instance->dateRegistration;}
    };
    return functionValues[index](this);
}
//----------------------------------------------------------
void VatNumberData::setValue(int index, const QVariant &value)
{
    static QList<std::function<void(VatNumberData *, const QVariant &)>> functionsSetValue = {
            [](VatNumberData *instance, const QVariant &value){ instance->number = value.toString();},
            [](VatNumberData *instance, const QVariant &value){ instance->dateRegistration = value.toDate();}
    };
functionsSetValue[index](this, value);
}
//----------------------------------------------------------
QString VatNumberData::countryCode() const
{
    QString code;
    if (number.size() > 2) {
        code = number.left(2);
    }
    return code;
}
//----------------------------------------------------------
QStringList VatNumberData::toStringList() const
{
    return {number, dateRegistration.toString("yyyy-MM-dd")};
}
//----------------------------------------------------------
VatNumberData VatNumberData::fromStringList(const QStringList &values)
{
    VatNumberData data;
    data.number = values[0];
    data.dateRegistration = QDate::fromString(values[1], "yyyy-MM-dd");
    return data;
}
//----------------------------------------------------------
QString VatNumberData::toString() const
{
    return toStringList().join(":::");
}
//----------------------------------------------------------
VatNumberData VatNumberData::fromString(const QString &value)
{
    return fromStringList(value.split(":::"));
}
//----------------------------------------------------------
bool VatNumberData::operator!=(const VatNumberData &other) const
{
    return number != other.number && dateRegistration != other.dateRegistration;
}
//----------------------------------------------------------

