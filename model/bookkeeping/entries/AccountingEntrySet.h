#ifndef ACCOUNTINGENTRYSET_H
#define ACCOUNTINGENTRYSET_H

#include <QtCore/qlist.h>
#include <QtCore/qsharedpointer.h>

#include "AccountingEntry.h"


class AccountingEntrySet
{
public:
    enum Type{Sale, Bank, Purchase, SaleMarketplace};
    enum State{Correct, Error, Edited};
    AccountingEntrySet(Type type);//Connection connectionType);
    bool isConnected() const;
    bool hasDocument() const;
    /*
    void connectTo(const QList<QSharedPointer<AccountingEntrySet>> &entrySets);
    void connectTo(QSharedPointer<AccountingEntrySet> &entrySet);
    void disconnect();
    //*/

    void load(const QString &filePath);
    void save(const QString &filePath) const;
    void updateOnAssociateTo(const AccountingEntrySet &other);
    void updateOnDissociateTo(const AccountingEntrySet &other);
    void updateOnSelfAssociateTo(const QString &title, const QString &account);
    void updateOnSelfDissociate();


    QString accountAssociation() const;
    QString fileRelWorkingDir() const;
    QString journal() const;
    QString label() const;
    QDate date() const;
    void setRateForConversion(double rate);
    double rateForConversion() const;
    double rateCurrencyDate() const;
    int year() const;
    int month() const;
    QString monthStr() const;
    int size() const;

    bool isDebit() const;
    /*
    double amountFirstOrigCur() const;
    double debitFirst() const;
    double creditFirst() const;
    //*/
    double debitTotalOrig() const;
    double creditTotalOrig() const;
    double debitTotalConv() const;
    double creditTotalConv() const;
    void roundCreditDebit();
    void roundCreditDebit(double toAddInHighestDebit, double toAddInHighestCredit);
    void addEntry(const AccountingEntry &entry);
    void insertEntry(int index, const AccountingEntry &entry);
    void replaceEntry(int index, const AccountingEntry &entry);
    void removeEntry(int index);
    void removeEntry(const QString &account);
    QList<AccountingEntry>::iterator entriesBegin();
    QList<AccountingEntry>::iterator entriesEnd();
    const QList<AccountingEntry> &entries() const;
    bool operator <(const AccountingEntrySet &other) const;

    QString sourceDocument() const;
    void setSourceDocument(const QString &sourceDocument);

    //Connection connection() const;

    bool hasHtmlDocument() const;
    QString htmlDocument() const;
    void setHtmlDocument(
            const QString &htmlDocument,
            const QString &docBaseName = QString());

    bool hasCsvData() const;
    const QString &csvFileBaseName() const;
    const QList<QStringList> &csvData() const;
    void setCsvData(
            const QList<QStringList> &csvData,
            const QString &csvFileBaseName);

    void select();
    void unselect();

    QString id() const;
    void setId(const QString &id);

    //QList<QSharedPointer<AccountingEntrySet> > connectedTo() const;

    QString sourceDocumentDisplay() const;
    void setSourceDocumentDisplay(const QString &sourceDocumentDisplay);


    double amountConv() const;
    double amountOrig() const;
    void setAmountOrig(double amountOrig);

    QString currencyOrig() const;
    void setCurrencyOrig(const QString &currencyOrig);

    Type type() const;

    State state() const;
    void setState(const State &state);

    QString htmlDocBaseName() const;

private:
    Type m_type;
    State m_state;
    QList<AccountingEntry> m_entries;
    QList<QSharedPointer<AccountingEntrySet>> m_connectedTo;
    QString m_sourceDocument;
    QString m_sourceDocumentDisplay;
    //Connection m_connection;
    QString m_htmlDocument;
    QString m_htmlDocBaseName;
    QList<QStringList> m_csvData;
    QString m_csvFileBaseName;
    bool m_selected;
    QString m_id;
    double m_amountOrig;
    QString m_currencyOrig;

};

//           Year     / journal      / month           / label
typedef QMap<int, QMap<QString, QMap<QString, QMultiMap<QString, QSharedPointer<AccountingEntrySet>>>>> AccountingEntries;
void removeEntryStatic(AccountingEntries &entries, QSharedPointer<AccountingEntrySet> entry);
void addEntryStatic(AccountingEntries &entries, QSharedPointer<AccountingEntrySet> entrySet);
void addEntriesStatic(AccountingEntries &entries, const AccountingEntries &entriesToAdd);

#endif // ACCOUNTINGENTRYSET_H
