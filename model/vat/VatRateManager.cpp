#include <QtCore/qsettings.h>

#include "model/SettingManager.h"

#include "VatRatesModel.h"
#include "VatRatesModelDates.h"
#include "SelectedSkusListModel.h"
#include "VatRateManager.h"
#include "model/CustomerManager.h"

//----------------------------------------------------------
VatRateManager::VatRateManager(QObject *)
{
    m_defautVatModel = new VatRatesModel("", this);
    m_defautVatModelDates = new VatRatesModelDates(this);
    QString selectedCustomerId = CustomerManager::instance()->getSelectedCustomerId();
    if (!selectedCustomerId.isEmpty()) {
        onCustomerSelectedChanged(selectedCustomerId);
    }
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            this,
            &VatRateManager::onCustomerSelectedChanged);
    QString settingKey = m_settingKey + "-FIRST-LAUNCH";
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!settings.contains(settingKey)) {
        QString rate0 = tr("Taux 0");
        addOtherRates(rate0);
        m_otherVatModels[rate0]->setUniqueValue(0.);
        m_otherSectedSkusModels[rate0]->addSku("INTERETBCA");
        settings.setValue(settingKey, true);
    }
}
//----------------------------------------------------------
void VatRateManager::onCustomerSelectedChanged(const QString &customerId)
{
    if (customerId.isEmpty()) {
        m_settingKey = "";
        clear();
    } else {
        m_settingKey = "VatRateManager-" + customerId;
        loadFromSettings();
    }
}
//----------------------------------------------------------
VatRateManager *VatRateManager::instance()
{
    static VatRateManager instance;
    return &instance;
}
//----------------------------------------------------------
double VatRateManager::vatRateDefault(
        const QString &countryCode, const QDate &date) const
{
    return m_defautVatModelDates->vatRate(
                countryCode, date,
                m_defautVatModel->vatRate(countryCode));
}
//----------------------------------------------------------
double VatRateManager::vatRate(
        const QString &countryCode, const QDate &date, const QString &sku) const
{
    if (sku == "INTERETBCA") {
        int TEMP=10;++TEMP;
    }
    for (auto it = m_otherSectedSkusModels.begin();
         it != m_otherSectedSkusModels.end();
         ++it) {
        if (it.value()->contains(sku)) {
            return m_otherVatModelsDate[it.key()]->vatRate(
                    countryCode, date,
                    m_otherVatModels[it.key()]->vatRate(countryCode));
        }
    }
    return vatRateDefault(countryCode, date);
}
//----------------------------------------------------------
VatRateManager::~VatRateManager()
{
}
//----------------------------------------------------------
VatRatesModel *VatRateManager::getDefautVatModel() const
{
    return m_defautVatModel;
}
//----------------------------------------------------------
VatRatesModelDates *VatRateManager::getDefautVatModelDates() const
{
    return m_defautVatModelDates;
}
//----------------------------------------------------------
QSharedPointer<VatRatesModel> VatRateManager::getOtherVatModel(
        const QString &name) const
{
    return m_otherVatModels[name];
}
//----------------------------------------------------------
QSharedPointer<VatRatesModelDates> VatRateManager::getOtherVatModelDate(
        const QString &name) const
{
    return m_otherVatModelsDate[name];
}
//----------------------------------------------------------
QSharedPointer<SelectedSkusListModel>
VatRateManager::getSelectedSkusModel(const QString &name) const
{
    return m_otherSectedSkusModels[name];
}
//----------------------------------------------------------
bool VatRateManager::containsVatModel(const QString &name) const
{
    return m_otherVatModels.contains(name);
}
//----------------------------------------------------------
void VatRateManager::addOtherRates(const QString &name)
{
    int position = m_otherVatModels.size();
    QString keyCustomRates
            = m_settingKey + "-" + QString::number(position);
    m_otherVatModels[name] = QSharedPointer<VatRatesModel>(
                new VatRatesModel(keyCustomRates));
    QString keyCustomRatesDate = keyCustomRates + "-date";
    m_otherVatModelsDate[name] = QSharedPointer<VatRatesModelDates>(
                new VatRatesModelDates());
    m_otherVatModelsDate[name]->setSettingKey(keyCustomRatesDate);
    QString keyCustomRatesSkus
            = m_settingKey + "-skus-" + QString::number(position);
    m_otherSectedSkusModels[name] = QSharedPointer<SelectedSkusListModel>(
                new SelectedSkusListModel(keyCustomRatesSkus));
    int index = m_otherVatModels.keys().indexOf(name);
    beginInsertRows(QModelIndex(), index, index);
    saveInSettings();
    endInsertRows();
}
//----------------------------------------------------------
void VatRateManager::remove(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    auto keys = m_otherVatModels.keys();
    m_otherVatModels.remove(keys[index]);
    m_otherVatModelsDate.remove(keys[index]);
    saveInSettings();
    endRemoveRows();
}
//----------------------------------------------------------
void VatRateManager::remove(const QString &name)
{
    int indexDelete = m_otherVatModels.keys().indexOf(name);
    beginRemoveRows(QModelIndex(), indexDelete, indexDelete);
    m_otherVatModels.remove(name);
    m_otherVatModelsDate.remove(name);
    saveInSettings();
    endRemoveRows();
}
//----------------------------------------------------------
void VatRateManager::loadFromSettings()
{
    clear();
    m_defautVatModel->loadFromSettings(m_settingKey);
    m_defautVatModelDates->loadFromSettings(m_settingKey + "-dates");
    QString keyNCustomRates = m_settingKey + "-nRates";
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(keyNCustomRates)) {
        int nCustomRates = settings.value(keyNCustomRates).toInt();
        beginInsertRows(QModelIndex(), 0, nCustomRates-1);
        for (int i=0; i<nCustomRates; i++) {
            int position = i + 1;
            QString keyCustomRatesName
                    = m_settingKey + "-name-" + QString::number(position);
            QString keyCustomRates
                    = m_settingKey + "-" + QString::number(position);
            QString keyCustomRatesSkus
                    = m_settingKey + "-skus-" + QString::number(position);
            QString name = settings.value(keyCustomRatesName).toString();
            m_otherVatModels[name] = QSharedPointer<VatRatesModel>(
                new VatRatesModel(keyCustomRates));
            m_otherVatModels[name]->loadFromSettings();
            QString keyCustomRatesDate = keyCustomRates + "-date";
            m_otherVatModelsDate[name] = QSharedPointer<VatRatesModelDates>(
                new VatRatesModelDates());
            m_otherVatModelsDate[name]->loadFromSettings(keyCustomRatesDate);
            m_otherSectedSkusModels[name]
                    = QSharedPointer<SelectedSkusListModel>(
                        new SelectedSkusListModel(keyCustomRatesSkus));
            m_otherSectedSkusModels[name]->loadFromSettings();
        }
        endInsertRows();
    }
}
//----------------------------------------------------------
void VatRateManager::saveInSettings() const
{
    //m_defautVatModel->saveValuesOfCustomer(keyId);
    int nCustomRates = m_otherVatModels.size();
    QString keyNCustomRates = m_settingKey + "-nRates";
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    settings.setValue(keyNCustomRates, nCustomRates);
    int position = 1;
    for(auto it=m_otherVatModels.begin();
        it != m_otherVatModels.end();
        ++it) {
        QString keyCustomRatesName
                = m_settingKey + "-name-" + QString::number(position);
        position++;
        settings.setValue(keyCustomRatesName, it.key());
    }
}
//----------------------------------------------------------
void VatRateManager::clear()
{
    if (m_otherVatModels.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_otherVatModels.size()-1);
        m_otherVatModels.clear();
        m_otherVatModelsDate.clear();
        m_otherSectedSkusModels.clear();
        endRemoveRows();
    }
}
//----------------------------------------------------------
int VatRateManager::rowCount(const QModelIndex &) const
{
    return m_otherVatModels.size();
}
//----------------------------------------------------------
QVariant VatRateManager::headerData(
        int section,
        Qt::Orientation orientation,
        int role) const
{
    (void) section;
    (void) orientation;
    (void) role;
    return QVariant();
}
//----------------------------------------------------------
QVariant VatRateManager::data(
        const QModelIndex &index, int role) const
{
    QVariant variant;
    if (role == Qt::DisplayRole) {
        variant = m_otherVatModels.keys()[index.row()];
    }
    return variant;
}
//----------------------------------------------------------
bool VatRateManager::setData(
        const QModelIndex &index,
        const QVariant &value,
        int role)
{
    bool change = false;
    (void) index;
    (void) value;
    (void) role;
    return change;
}
//----------------------------------------------------------
