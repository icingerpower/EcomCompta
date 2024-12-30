#ifndef ENTRYPARSERPURCHASES_H
#define ENTRYPARSERPURCHASES_H

#include <QtCore/qdir.h>
#include <QtCore/qexception.h>

#include "AbstractEntryParser.h"

struct PurchaseInvoiceInfo{
    QString absFileName; /// Can be either abs / or relative to invoice directory
    QDate date;
    QString label;
    QString account;
    QString accountSupplier;
    double amount;
    QString currency;
    double amountVat;
    QString currencyVat;
    QString vatCountryName;
    QString vatCountryCode;
};

class EntryParserPurchases
        : public AbstractEntryParser
{
public:
    static PurchaseInvoiceInfo invoiceInfoFromFileName(const QString &fileName);
    static QString invoiceInfoToFileName(const PurchaseInvoiceInfo &infos);

    EntryParserPurchases();
    AccountingEntries entries(int year) const override;
    QSharedPointer<AccountingEntrySet> addInvoice(
            const PurchaseInvoiceInfo &invoiceInfo);
    void removeInvoice(int year, int month, const QString &id) const;
    QSharedPointer<AccountingEntrySet> updateInvoice(const PurchaseInvoiceInfo &invoiceInfo) const;
    /*
    const QString &absFileName,
                    const QDate &date, const QString &label,
                    const QString &account, const QString &supplier,
                    double amount,
                    const QString &currency,
                    double amountVat,
                    const QString &currencyVat,
                    const QString &vatCountry);
                    //*/
    Type typeOfEntries() const override;
    QString name() const override;
    QString journal() const override;
    QSharedPointer<AccountingEntrySet> entrySetFromFileName(
            const QString &relFileName) const;

    static QString genPurchaseId(
            const QDate &date, const QString &label, const QString &amountTaxed, const QString &currency);
};

class EntryParserPurchasesException : public QException
{
public:
    void raise() const override;
    EntryParserPurchasesException *clone() const override;

    QString error() const;
    void setError(const QString &error);

private:
    QString m_error;
};

#endif // ENTRYPARSERPURCHASES_H
