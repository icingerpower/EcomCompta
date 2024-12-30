#ifndef SERVICEACCOUNTS_H
#define SERVICEACCOUNTS_H

#include <QString>

class ServiceAccounts
{
public:
    static QString KEY_SETTINGS_SERVICE_ACCOUNTS_SALE;
    static QString KEY_SETTINGS_SERVICE_ACCOUNTS_VAT_TO_DECLARE;
    static QString KEY_SETTINGS_SERVICE_ACCOUNTS_VAT_COLLECTED;
    static QString KEY_SETTINGS_SERVICE_ACCOUNTS_CLIENT;
    ServiceAccounts(const QString &clientId);
    bool hasVatToDeclareOnPayment() const;

    QString accountSale() const;
    QString accountSale(const QString &defaultAccount) const;
    void setAccountSale(const QString &newAccountSale);

    QString accountVatToDeclare() const;
    QString accountVatToDeclare(const QString &defaultAccount) const;
    void setAccountVatToDeclare(const QString &newAccountVatToDeclare);

    QString accountVatCollected() const;
    QString accountVatCollected(const QString &defaultAccount) const;
    void setAccountVatCollected(const QString &newAccountVatCollected);

    QString accountClient() const;
    QString accountClient(const QString &defaultAccount) const;
    void setAccountClient(const QString &newAccountClient);

protected:
    bool m_taxeOnPayment;
    QString m_accountSale;
    QString m_accountVatToDeclare;
    QString m_accountVatCollected;
    QString m_accountClient;
    void loadFromSettings();
    void saveInSettings();
    QString m_clientId;
    inline QString _settingKey(const QString &attrKey) const;
};

#endif // SERVICEACCOUNTS_H
