#include <QSettings>

#include "model/SettingManager.h"

#include "ServiceAccounts.h"

//==========================================================
QString ServiceAccounts::KEY_SETTINGS_SERVICE_ACCOUNTS_SALE = "service-accounts-erasing-sale";
QString ServiceAccounts::KEY_SETTINGS_SERVICE_ACCOUNTS_VAT_TO_DECLARE = "service-accounts-erasing-vat-to-declare";
QString ServiceAccounts::KEY_SETTINGS_SERVICE_ACCOUNTS_VAT_COLLECTED = "service-accounts-erasing-vat-collected";
QString ServiceAccounts::KEY_SETTINGS_SERVICE_ACCOUNTS_CLIENT = "service-accounts-erasing-client";
//==========================================================
ServiceAccounts::ServiceAccounts(const QString &clientId)
{
    m_clientId = clientId;
    loadFromSettings();
}
//==========================================================
bool ServiceAccounts::hasVatToDeclareOnPayment() const
{
    return !m_accountVatToDeclare.isEmpty();
}
//==========================================================
QString ServiceAccounts::accountSale() const
{
    return m_accountSale;
}
//==========================================================
QString ServiceAccounts::accountSale(
        const QString &defaultAccount) const
{
    return m_accountSale.isEmpty() ? defaultAccount : m_accountSale;
}
//==========================================================
void ServiceAccounts::setAccountSale(
        const QString &newAccountSale)
{
    m_accountSale = newAccountSale;
    saveInSettings();
}
//==========================================================
QString ServiceAccounts::accountVatToDeclare() const
{
    return m_accountVatToDeclare;
}
//==========================================================
QString ServiceAccounts::accountVatToDeclare(
        const QString &defaultAccount) const
{
    return m_accountVatToDeclare.isEmpty() ? defaultAccount : m_accountVatToDeclare;
}
//==========================================================
void ServiceAccounts::setAccountVatToDeclare(
        const QString &newAccountVatToDeclare)
{
    m_accountVatToDeclare = newAccountVatToDeclare;
    saveInSettings();
}
//==========================================================
QString ServiceAccounts::accountVatCollected() const
{
    return m_accountVatCollected;
}
//==========================================================
QString ServiceAccounts::accountVatCollected(
        const QString &defaultAccount) const
{
    return m_accountVatCollected.isEmpty() ? defaultAccount : m_accountVatCollected;
}
//==========================================================
void ServiceAccounts::setAccountVatCollected(
        const QString &newAccountVatCollected)
{
    m_accountVatCollected = newAccountVatCollected;
    saveInSettings();
}
//==========================================================
QString ServiceAccounts::accountClient() const
{
    return m_accountClient;
}
//==========================================================
QString ServiceAccounts::accountClient(
        const QString &defaultAccount) const
{
    return m_accountClient.isEmpty() ? defaultAccount : m_accountClient;
}
//==========================================================
void ServiceAccounts::setAccountClient(
        const QString &newAccountClient)
{
    m_accountClient = newAccountClient;
    saveInSettings();
}
//==========================================================
void ServiceAccounts::loadFromSettings()
{
    QSettings settings(
                SettingManager::instance()->settingsFilePath(),
                QSettings::IniFormat);
    if (settings.contains(
                _settingKey(KEY_SETTINGS_SERVICE_ACCOUNTS_SALE))) {
        m_accountSale = settings.value(
                    _settingKey(KEY_SETTINGS_SERVICE_ACCOUNTS_SALE)).toString();
    }
    if (settings.contains(
                _settingKey(KEY_SETTINGS_SERVICE_ACCOUNTS_VAT_TO_DECLARE))) {
        m_accountVatToDeclare = settings.value(
                    _settingKey(KEY_SETTINGS_SERVICE_ACCOUNTS_VAT_TO_DECLARE)).toString();
    }
    if (settings.contains(
                _settingKey(KEY_SETTINGS_SERVICE_ACCOUNTS_VAT_COLLECTED))) {
        m_accountVatCollected = settings.value(
                    _settingKey(KEY_SETTINGS_SERVICE_ACCOUNTS_VAT_COLLECTED)).toString();
    }
    if (settings.contains(
                _settingKey(KEY_SETTINGS_SERVICE_ACCOUNTS_CLIENT))) {
        m_accountClient = settings.value(
                    _settingKey(KEY_SETTINGS_SERVICE_ACCOUNTS_CLIENT)).toString();
    }
}
//==========================================================
void ServiceAccounts::saveInSettings()
{
    QSettings settings(
                SettingManager::instance()->settingsFilePath(),
                QSettings::IniFormat);
    QString settingKeySale = _settingKey(KEY_SETTINGS_SERVICE_ACCOUNTS_SALE);
    if (!m_accountSale.isEmpty()) {
        settings.setValue(
                    settingKeySale,
                    m_accountSale);
    } else if (settings.contains(settingKeySale)) {
        settings.remove(settingKeySale);
    }

    QString settingKeySaleVatCollected = _settingKey(
                KEY_SETTINGS_SERVICE_ACCOUNTS_VAT_COLLECTED);
    if (!m_accountVatCollected.isEmpty()) {
        settings.setValue(
                    settingKeySaleVatCollected,
                    m_accountVatCollected);
    } else if (settings.contains(settingKeySaleVatCollected)) {
        settings.remove(settingKeySaleVatCollected);
    }

    QString settingKeyClient = _settingKey(
                KEY_SETTINGS_SERVICE_ACCOUNTS_CLIENT);
    if (!m_accountClient.isEmpty()) {
        settings.setValue(
                    settingKeyClient,
                    m_accountClient);
    } else if (settings.contains(settingKeyClient)) {
        settings.remove(settingKeyClient);
    }

    QString settingKeySaleVatToDeclare = _settingKey(
                KEY_SETTINGS_SERVICE_ACCOUNTS_VAT_TO_DECLARE);
    if (!m_accountVatToDeclare.isEmpty()) {
        settings.setValue(
                    settingKeySaleVatToDeclare,
                    m_accountVatToDeclare);
    } else if (settings.contains(settingKeySaleVatToDeclare)) {
        settings.remove(settingKeySaleVatToDeclare);
    }
}
//==========================================================
QString ServiceAccounts::_settingKey(
        const QString &attrKey) const
{
    QString string(m_clientId);
    string += "-";
    string += attrKey;
    return string;
}
//==========================================================

