#include <QtCore/qsettings.h>

#include "VatThresholdModel.h"
#include "model/SettingManager.h"
#include "model/CustomerManager.h"

//----------------------------------------------------------
VatThresholdModel::VatThresholdModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    initWithDefaultValues();
    QString selectedCustomerId = CustomerManager::instance()->getSelectedCustomerId();
    if (!selectedCustomerId.isEmpty()) {
        onCustomerSelectedChanged(selectedCustomerId);
    }
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            this,
            &VatThresholdModel::onCustomerSelectedChanged);
}
//----------------------------------------------------------
VatThresholdModel *VatThresholdModel::instance()
{
    static VatThresholdModel instance;
    return  &instance;
}
//----------------------------------------------------------
void VatThresholdModel::initWithDefaultValues()
{
    /*
    m_countries = *SettingManager::countriesUE();
    for (auto country : m_countries) {
        m_countryChoices[country] = CHOICE::ALWAYS;
        m_thresholds[country] = 0.;
    }
    //*/
}
//----------------------------------------------------------
void VatThresholdModel::saveInSettings() const
{
    if (!m_settingKey.isEmpty()) {
        QSettings settings(SettingManager::instance()->settingsFilePath(),
                           QSettings::IniFormat);
        QStringList elements;
        for (auto country : m_countries) {
            QString element = country + ",";
            element += QString::number(m_countryChoices[country]) + ",";
            element += QString::number(m_thresholds[country]);
            elements << element;
        }
        QString stringForSetting = elements.join("::");
        settings.setValue(m_settingKey, stringForSetting);
    }
}
//----------------------------------------------------------
void VatThresholdModel::loadFromSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(m_settingKey)) {
        QString stringForSetting = settings.value(m_settingKey).toString();
        QStringList elements = stringForSetting.split("::");
        for (auto element : elements) {
            QStringList values = element.split(",");
            QString country = values[0];
            QString choiceString = values[1];
            QString threshold = values[2];
            m_countryChoices[country] = static_cast<CHOICE>(choiceString.toInt());
            m_thresholds[country] = threshold.toDouble();
        }
    } else {
        initWithDefaultValues();
    }
    emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
}
//----------------------------------------------------------
double VatThresholdModel::threshold(const QString &country) const
{
    double value = 0.;
    if (m_countryChoices[country] == THRESHOLD) {
        value = 10000.;
    } else if (m_countryChoices[country] == CUSTOM_THRESHOLD) {
        value = m_thresholds[country];
    }
    return value;
}
//----------------------------------------------------------
QVariant VatThresholdModel::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    QVariant data;
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            static QStringList header = {"", "", "", "Seuil personnalisé"};
            data = header[section];
        } else {
            data = m_countries[section];
        }
    }
    return data;
}
//----------------------------------------------------------
int VatThresholdModel::rowCount(const QModelIndex &) const
{
    return m_countries.size();
}
//----------------------------------------------------------
int VatThresholdModel::columnCount(const QModelIndex &) const
{
    return 4;
}
//----------------------------------------------------------
Qt::ItemFlags VatThresholdModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    if (index.column() < 3) {
        flags |= Qt::ItemIsUserCheckable;
    } else {
         QString country = m_countries[index.row()];
         if (m_countryChoices[country] == CHOICE::CUSTOM_THRESHOLD) {
             flags |= Qt::ItemIsEditable;
         } else {
             flags = Qt::NoItemFlags;
         }
    }
    return flags;
}
//----------------------------------------------------------
QVariant VatThresholdModel::data(const QModelIndex &index, int role) const
{
    QVariant data;
    if (role == Qt::DisplayRole) {
        if (index.column() < 3) {
            static QStringList values = {"TVA dès 0 €", "TVA dès 10 000 €", "TVA dès seuil personnalisé"};
            data = values[index.column()];
        } else {
            QString country = m_countries[index.row()];
            //if (m_countryChoices[country] == CHOICE::CUSTOM_THRESHOLD) {
            data = m_thresholds[country];
            //}
        }
    } else if (role == Qt::EditRole && index.column() == 3) {
        QString country = m_countries[index.row()];
        data = m_thresholds[country];
    } else if (role == Qt::CheckStateRole && index.column() < 3) {
        QString country = m_countries[index.row()];
        data = m_countryChoices[country] == index.column();
    }
    return data;
}
//----------------------------------------------------------
bool VatThresholdModel::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    bool edited = false;
    if (role == Qt::EditRole && index.column() == 3) {
        QString country = m_countries[index.row()];
        m_thresholds[country] = value.toDouble();
        edited = true;
    } else if (role == Qt::CheckStateRole) {
        QString country = m_countries[index.row()];
        bool notChecked = m_countryChoices[country] != index.column();
        if (notChecked && value.toBool()) {
            CHOICE beforeChoice = m_countryChoices[country];
            m_countryChoices[country] = static_cast<CHOICE>(index.column());
            QModelIndex indexChanged = this->index(
                        index.row(), beforeChoice, QModelIndex());
            emit dataChanged(indexChanged, indexChanged, {Qt::CheckStateRole});
            QModelIndex indexChangedThreshold = this->index(
                    index.row(), 3, QModelIndex());
            emit dataChanged(indexChangedThreshold, indexChangedThreshold, {Qt::DisplayRole});
            edited = true;
        }
    }
    if (edited) {
        saveInSettings();
    }
    return edited;
}
//----------------------------------------------------------
void VatThresholdModel::onCustomerSelectedChanged(const QString &customerId)
{
    if (customerId.isEmpty()) {
        m_settingKey = "";
        initWithDefaultValues();
        emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
    } else {
        m_settingKey = "VatThresholdModel-" + customerId;
        loadFromSettings();
    }
}
//----------------------------------------------------------
