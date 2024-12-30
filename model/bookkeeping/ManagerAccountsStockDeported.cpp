#include <QSharedPointer>
#include <QSettings>
#include <QDate>

#include "../common/countries/CountryManager.h"

#include "model/SettingManager.h"
#include "ExceptionAccountDeportedMissing.h"

#include "ManagerAccountsStockDeported.h"

//----------------------------------------------------------
QStringList ManagerAccountsStockDeported::COL_NAMES
= {QObject::tr("Pays")
   , QObject::tr("Compte 6 acquisition")
   , QObject::tr("Compte 7 vente intracom")
  };
int ManagerAccountsStockDeported::IND_COL_ACCOUNT_6_IMPORTED = 1;
int ManagerAccountsStockDeported::IND_COL_ACCOUNT_7_EXPORTED = 2;
QString ManagerAccountsStockDeported::KEY_ACCOUNT_4 = "account-stock-deported-4";
QString ManagerAccountsStockDeported::KEY_ACCOUNT_4_VAT_TO_PAY = "account-stock-deported-4-vat-to-pay";
QString ManagerAccountsStockDeported::KEY_ACCOUNT_4_VAT_DEDUCTIBLE = "account-stock-deported-4-vat-deductible";;
QString ManagerAccountsStockDeported::KEY_ACCOUNT_4_OUTSIDE_COUNTRY = "account-stock-deported-4-outside-country-company";;
//----------------------------------------------------------
ManagerAccountsStockDeported::ManagerAccountsStockDeported(
        QObject *parent)
    : QAbstractTableModel(parent)
{
    auto countryNamesUeSorted = CountryManager::instance()
            ->countriesNamesUE(
                QDate::currentDate().year());
    int i=0;
    for (auto itCountry = countryNamesUeSorted->begin();
         itCountry != countryNamesUeSorted->end(); ++itCountry) {
        m_listOfStringList << QStringList(
                                   {*itCountry
                                   , QString()
                                   , QString()
                                   });
        QString countryCode = CountryManager::instance()
                ->countryCode(*itCountry);
        m_countryCodeToIndex[countryCode] = i;
        ++i;
    }
}
//----------------------------------------------------------
void ManagerAccountsStockDeported::_clear()
{
    beginRemoveRows(QModelIndex(), 0, rowCount()-1);
    m_listOfStringList.clear();
    endRemoveRows();
}
//----------------------------------------------------------
void ManagerAccountsStockDeported::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!settingKey().isEmpty()) {
        QStringList lines;
        for (auto itLine = m_listOfStringList.begin();
             itLine != m_listOfStringList.end(); ++itLine) {
            QStringList lineElements;
            for (auto itEl = itLine->begin();
                 itEl != itLine->end(); ++itEl) {
                lineElements << *itEl;
            }
            lines << lineElements.join(";;;");
        }
        settings.setValue(
                    settingKey(),
                    lines.join(":::"));
    }
}
//----------------------------------------------------------
void ManagerAccountsStockDeported::loadFromSettings()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!settingKey().isEmpty()) {
        if (settings.contains(settingKey())) {
            //_clear();
            QList<QStringList> loadedValues;
            auto loadedString = settings.value(settingKey()).toString();
            QStringList lines = loadedString.split(":::");
            for (auto itLine = lines.begin();
                 itLine != lines.end(); ++itLine) {
                loadedValues << itLine->split(";;;");
            }

            for (auto itLoaded = loadedValues.begin();
                 itLoaded != loadedValues.end(); ++itLoaded) {
                for (auto itVal = m_listOfStringList.begin();
                     itVal != m_listOfStringList.end(); ++itVal) {
                    if (itLoaded->value(0) == itVal->value(0)) {
                        *itVal = *itLoaded;
                    }
                }
            }
            emit dataChanged(index(0, 0),
                             index(rowCount()-1, columnCount()-1));
        }
    }
}
//----------------------------------------------------------
ManagerAccountsStockDeported *ManagerAccountsStockDeported::instance()
{
    static QSharedPointer<ManagerAccountsStockDeported> instance
            = []() -> QSharedPointer<ManagerAccountsStockDeported>{
            QSharedPointer<ManagerAccountsStockDeported> _instance(
                new ManagerAccountsStockDeported);
            _instance->init();
            return _instance;
}();
    return instance.data();
}
//----------------------------------------------------------
QString ManagerAccountsStockDeported::getAccountImportedFromUe(
        const QString &countryCode) const
{
    Q_ASSERT(m_countryCodeToIndex.contains(countryCode));
    int index = m_countryCodeToIndex[countryCode];
    QString account = m_listOfStringList[index][IND_COL_ACCOUNT_6_IMPORTED];
    if (account.isEmpty()) {
        ExceptionAccountDeportedMissing exception;
        exception.setCountry(
                    CountryManager::instance()->countryName(
                        countryCode));
        exception.raise();
    }
    return account;
}
//----------------------------------------------------------
QString ManagerAccountsStockDeported::getAccountExportedToUe(
        const QString &countryCode) const
{
    Q_ASSERT(m_countryCodeToIndex.contains(countryCode));
    int index = m_countryCodeToIndex[countryCode];
    QString account = m_listOfStringList[index][IND_COL_ACCOUNT_7_EXPORTED];
    if (account.isEmpty()) {
        ExceptionAccountDeportedMissing exception;
        exception.setCountry(
                    CountryManager::instance()->countryName(
                        countryCode));
        exception.raise();
    }
    return account;
}
//----------------------------------------------------------
QString ManagerAccountsStockDeported::getAccount4()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    return settings.value(KEY_ACCOUNT_4, QString("467800")).toString();
}
//----------------------------------------------------------
QString ManagerAccountsStockDeported::getAccount4VatToPay()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    return settings.value(KEY_ACCOUNT_4_VAT_TO_PAY, QString("445200")).toString();
}
//----------------------------------------------------------
void ManagerAccountsStockDeported::setAccount4VatToPay(
        const QString &account)
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    settings.setValue(KEY_ACCOUNT_4_VAT_TO_PAY, account);
}
//----------------------------------------------------------
QString ManagerAccountsStockDeported::getAccount4VatDeductible()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    return settings.value(
                KEY_ACCOUNT_4_VAT_DEDUCTIBLE,
                QString("445664")).toString();
}
//----------------------------------------------------------
QString ManagerAccountsStockDeported::getAccount4OutsideCountry()
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    return settings.value(
                KEY_ACCOUNT_4_OUTSIDE_COUNTRY,
                QString("467810")).toString();
}
//----------------------------------------------------------
void ManagerAccountsStockDeported::setAccount4OutsideCountry(
        const QString &account)
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    settings.setValue(KEY_ACCOUNT_4_OUTSIDE_COUNTRY, account);
}
//----------------------------------------------------------
void ManagerAccountsStockDeported::setAccount4VatDeductible(
        const QString &account)
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    settings.setValue(KEY_ACCOUNT_4_VAT_DEDUCTIBLE, account);
}
//----------------------------------------------------------
void ManagerAccountsStockDeported::setAccount4(const QString &account)
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    settings.setValue(KEY_ACCOUNT_4, account);
}
//----------------------------------------------------------
void ManagerAccountsStockDeported::onCustomerSelectedChanged(
        const QString &)
{
    //UpdateToCustomer::onCustomerSelectedChanged(customerId);
    loadFromSettings();
}
//----------------------------------------------------------
QString ManagerAccountsStockDeported::uniqueId() const
{
    return QString("ManagerAccountsStockDeported");
}
//----------------------------------------------------------
QVariant ManagerAccountsStockDeported::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return COL_NAMES[section];
    }
    return QVariant();
}
//----------------------------------------------------------
int ManagerAccountsStockDeported::rowCount(
        const QModelIndex &) const
{
    return m_listOfStringList.size();
}
//----------------------------------------------------------
int ManagerAccountsStockDeported::columnCount(
        const QModelIndex &) const
{
    return COL_NAMES.size();
}
//----------------------------------------------------------
QVariant ManagerAccountsStockDeported::data(
        const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_listOfStringList[index.row()][index.column()];
    }
    return QVariant();
}
//----------------------------------------------------------
bool ManagerAccountsStockDeported::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        m_listOfStringList[index.row()][index.column()] = value.toString();
        saveInSettings();
        return true;
    }
    return false;
}
//----------------------------------------------------------
Qt::ItemFlags ManagerAccountsStockDeported::flags(
        const QModelIndex &index) const
{
    if (index.column() > 0) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
//----------------------------------------------------------
