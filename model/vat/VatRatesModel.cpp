#include <QtCore/qsettings.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qstring.h>

#include "VatRatesModel.h"
#include "model/SettingManager.h"


//----------------------------------------------------------
VatRatesModel::VatRatesModel(
        const QString &settingKey, QObject *parent)
    : QAbstractTableModel (parent)
{
    m_settingKey = settingKey;
    initWithDefaultValues();
}
//----------------------------------------------------------
VatRatesModel::~VatRatesModel()
{

}
//----------------------------------------------------------
void VatRatesModel::initWithDefaultValues()
{
    m_countries = *SettingManager::countriesUEfrom2020();
    m_vatRates["FR"] = 0.2;
    m_vatRates["GB"] = 0.2;
    m_vatRates["GB-NIR"] = 0.2;
    m_vatRates["DE"] = 0.19;
    m_vatRates["IT"] = 0.22;
    m_vatRates["ES"] = 0.21;
    m_vatRates["PL"] = 0.23;
    m_vatRates["CZ"] = 0.21;
    m_vatRates["NL"] = 0.21;
    m_vatRates["SE"] = 0.25;
    m_vatRates["BE"] = 0.21;
    m_vatRates["BG"] = 0.2;
    m_vatRates["DK"] = 0.25;
    m_vatRates["EE"] = 0.2;
    m_vatRates["IE"] = 0.23;
    m_vatRates["EL"] = 0.24;
    m_vatRates["HR"] = 0.25;
    m_vatRates["CY"] = 0.19;
    m_vatRates["LV"] = 0.21;
    m_vatRates["LT"] = 0.21;
    m_vatRates["LU"] = 0.17;
    m_vatRates["HU"] = 0.27;
    m_vatRates["MT"] = 0.18;
    m_vatRates["AT"] = 0.2;
    m_vatRates["PT"] = 0.23;
    m_vatRates["RO"] = 0.19;
    m_vatRates["SI"] = 0.22;
    m_vatRates["SK"] = 0.2;
    m_vatRates["FI"] = 0.24;
    for (int i=m_countries.size()-1; i>=0; --i) {
        if (!m_vatRates.contains(m_countries[i])) {
            m_countries.removeAt(i);
        }
    }
}
//----------------------------------------------------------
void VatRatesModel::loadFromSettings(const QString &settingKey)
{
    m_settingKey = settingKey;
    loadFromSettings();
}
//----------------------------------------------------------
void VatRatesModel::loadFromSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(m_settingKey)) {
        QString valuesString = settings.value(m_settingKey).toString();
        for (auto val : valuesString.split(";")) {
            QStringList elements = val.split("=");
            if (m_vatRates.contains(elements[0])) {
                m_vatRates[elements[0]] = elements[1].toDouble();
            }
        }
    } else {
        initWithDefaultValues();
    }
    emit dataChanged(index(0,0), index(0,m_countries.length()));
}
//----------------------------------------------------------
void VatRatesModel::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    QStringList elements;
    for (auto it=m_vatRates.begin();
         it!=m_vatRates.end();
         ++it) {
        elements << it.key() + "=" + QString::number(it.value());
    }
    if (elements.size() > 0) {
        settings.setValue(m_settingKey, elements.join(";"));
    } else if (settings.contains(m_settingKey)){
        settings.remove(m_settingKey);
    }
}
//----------------------------------------------------------
QHash<QString, double> VatRatesModel::getVatRates() const
{
    return m_vatRates;
}
//----------------------------------------------------------
double VatRatesModel::vatRate(const QString &countryCode) const
{
    if (countryCode == "GR") {
        return m_vatRates["EL"];
    } else if (countryCode == "MC") {
        return m_vatRates["FR"];
    } else if (countryCode == "UK") {
        return m_vatRates["GB"];
    }
    Q_ASSERT(m_vatRates.contains(countryCode));
    return m_vatRates[countryCode];
}
//----------------------------------------------------------
Qt::ItemFlags VatRatesModel::flags(const QModelIndex &) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled
            | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    return flags;
}
//----------------------------------------------------------
QStringList VatRatesModel::countries() const
{
    return m_countries;
}
//----------------------------------------------------------
int VatRatesModel::rowCount(const QModelIndex &) const
{
    return 1;
}
//----------------------------------------------------------
int VatRatesModel::columnCount(const QModelIndex &) const
{
    return m_countries.size();
}
//----------------------------------------------------------
QVariant VatRatesModel::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            value = m_countries[section];
        } else {
            value = "Taux de TVA";
        }
    }
    return value;
}
//----------------------------------------------------------
QVariant VatRatesModel::data(const QModelIndex &index, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        value = m_vatRates[m_countries[index.column()]];
    }
    return value;
}
//----------------------------------------------------------
bool VatRatesModel::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    bool change = false;
    if (role == Qt::EditRole) {
        QString country = m_countries[index.column()];
        m_vatRates[country] = value.toDouble();
        saveInSettings();
    }
    return change;
}
//----------------------------------------------------------
void VatRatesModel::setUniqueValue(double rate)
{
    for (auto it = m_vatRates.begin(); it!= m_vatRates.end(); ++it) {
        m_vatRates[it.key()] = rate;
        emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
    }
    saveInSettings();
}
//----------------------------------------------------------

