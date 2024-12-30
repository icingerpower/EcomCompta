#ifndef ENTRYPARSERIMPORTATIONS_H
#define ENTRYPARSERIMPORTATIONS_H

#include <QtCore/qdir.h>

#include "AbstractEntryParser.h"

/// 2023-01-06__626300__FR__DE__expedition-import-DE-030__FTIKSU__2593USD.pdf
struct ImportInvoiceInfo{
    QString absFileName; /// Can be either abs / or relative to invoice directory
    QDate date;
    QString label;
    QString accountOrig6;
    QString countryCodeFrom;
    QString countryCodeTo;
    double amount;
    QString currency;
    double amountVat;
    QString currencyVat;
    QString vatCountryName;
    QString vatCountryCode;
};

class EntryParserImportations
        : public AbstractEntryParser
{
public:
    static ImportInvoiceInfo invoiceInfoFromFileName(const QString &fileName);
    static QString invoiceInfoToFileName(const ImportInvoiceInfo &infos);

    AccountingEntries entries(int year) const;

    EntryParserImportations();
    Type typeOfEntries() const override;
    QString name() const override;
    QString journal() const override;

    QSharedPointer<AccountingEntrySet> addInvoice(
            const ImportInvoiceInfo &invoiceInfo);
    void removeInvoice(int year, int month, const QString &id) const;
    QSharedPointer<AccountingEntrySet> updateInvoice(
            const ImportInvoiceInfo &invoiceInfos) const;
    QSharedPointer<AccountingEntrySet> entrySetFromFileName(
            const QString &relFileName) const;

};

#endif // ENTRYPARSERIMPORTATIONS_H
