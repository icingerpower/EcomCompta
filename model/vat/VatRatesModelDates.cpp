#include <QtCore/qsettings.h>

#include "../common/gui/delegates/DelegateCountries.h"
#include "../common/countries/CountryManager.h"

#include "VatRatesModelDates.h"
#include "model/vat/ManagerCompanyVatParams.h"
#include "model/vat/VatRateManager.h"
#include "model/vat/VatRatesModel.h"
#include "model/SettingManager.h"

//----------------------------------------------------------
VatRatesModelDates::VatRatesModelDates(QObject *parent)
    : QAbstractTableModel(parent)
{
}
//----------------------------------------------------------
void VatRatesModelDates::add()
{
    QDate today = QDate::currentDate();
    QString companyCountry = ManagerCompanyVatParams::instance()->countryNameCompany();
    QString companyCountryCode = ManagerCompanyVatParams::instance()->countryCodeCompany();
    QVariantList values
            = {today.addDays(-1),
               today,
               companyCountry,
               VatRateManager::instance()->getDefautVatModel()->vatRate(companyCountryCode)};
    beginInsertRows(QModelIndex(), 0, 0);
    m_values.insert(0, values);
    _generateValuesByCountry();
    endInsertRows();
}
//----------------------------------------------------------
void VatRatesModelDates::deleteRow(const QModelIndex &index)
{
    deleteRow(index.row());
}
//----------------------------------------------------------
void VatRatesModelDates::deleteRow(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    m_values.removeAt(index);
    _generateValuesByCountry();
    saveInSettings();
    endRemoveRows();
}
//----------------------------------------------------------
double VatRatesModelDates::vatRate(
        const QString &country,
        const QDate &date,
        double defaultRate) const
{
    if (m_valuesByCounty.contains(country)) {
        for (auto rates : m_valuesByCounty.values(country)) {
            if (date >= rates.begin && date <= rates.end) {
                return rates.rate;
            }
        }
    }
    return defaultRate;
}
//----------------------------------------------------------
void VatRatesModelDates::loadFromSettings(const QString &settingKey)
{
    m_settingKey = settingKey;
    loadFromSettings();
}
//----------------------------------------------------------
void VatRatesModelDates::loadFromSettings()
{
    if (m_values.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_values.size()-1);
        m_values.clear();
        m_valuesByCounty.clear();
        endRemoveRows();
    }
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(m_settingKey)) {
        QString valuesString = settings.value(m_settingKey).toString();
        QStringList lines = valuesString.split(";;");
        for (auto line : lines) {
            QStringList elements = line.split("::");
            QVariantList variants;
            variants << QDate::fromString(elements[0], "yyyy-MM-dd");
            variants << QDate::fromString(elements[1], "yyyy-MM-dd");
            variants << elements[2];
            variants << elements[3].toDouble();
            m_values << variants;
        }
    }
    if (m_values.size() > 0) {
        beginInsertRows(QModelIndex(), 0, m_values.size()-1);
        _generateValuesByCountry();
        endInsertRows();
    }
}
//----------------------------------------------------------
void VatRatesModelDates::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (m_values.size() > 0) {
        QStringList lines;
        QString sep = "::";
        for (auto elements : m_values) {
            QString line = elements[0].toDate().toString("yyyy-MM-dd");
            line += sep + elements[1].toDate().toString("yyyy-MM-dd");
            line += sep + elements[2].toString();
            line += sep + elements[3].toString();
            lines << line;
        }
        settings.setValue(m_settingKey, lines.join(";;"));
    } else if (settings.contains(m_settingKey)) {
        settings.remove(m_settingKey);
    }
}
//----------------------------------------------------------
QVariant VatRatesModelDates::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QVariantList values
                = {tr("Début"), tr("Fin"), tr("Pays"), tr("Taux de TVA")};
        return values[section];
    }
    return QVariant();
}
//----------------------------------------------------------
Qt::ItemFlags VatRatesModelDates::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
}
//----------------------------------------------------------
int VatRatesModelDates::rowCount(const QModelIndex &) const
{
    return m_values.size();
}
//----------------------------------------------------------
int VatRatesModelDates::columnCount(const QModelIndex &) const
{
    return 4;
}
//----------------------------------------------------------
QVariant VatRatesModelDates::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_values[index.row()][index.column()];
    }
    return QVariant();
}
//----------------------------------------------------------
bool VatRatesModelDates::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        if (m_values[index.row()][index.column()] != value) {
            m_values[index.row()][index.column()] = value;
            _generateValuesByCountry();
            saveInSettings();
            return true;
        }
    }
    return false;
}

void VatRatesModelDates::setSettingKey(const QString &settingKey)
{
    m_settingKey = settingKey;
}
//----------------------------------------------------------
void VatRatesModelDates::_generateValuesByCountry()
{
    m_valuesByCounty.clear();
    for (auto elements : m_values) {
        QString countryName = elements[2].toString();
        QString countryCode = CountryManager::instance()->countryCode(countryName);
        m_valuesByCounty.insert(countryCode,
                {elements[0].toDate()
                , elements[1].toDate()
                , elements[3].toDouble()});
    }
}
//----------------------------------------------------------
