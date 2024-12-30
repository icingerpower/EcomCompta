#ifndef ENTRYPARSERBANK_H
#define ENTRYPARSERBANK_H

#include <QtCore/qexception.h>
#include <QtCore/qvariant.h>
#include <QtCore/qstring.h>
#include "AbstractEntryParser.h"

class EntryParserBank
        : public AbstractEntryParser
{
public:
    EntryParserBank();
    AccountingEntries entries(int year) const override;
    Type typeOfEntries() const override;
    QString nameFolder() const;
    QString fileName(int year, int month) const;
    QString fileName(const QString &year, const QString &month) const;
    QString baseName(int year, int month) const;
    QString baseName(const QString &year, const QString &month) const;
    QList<QSharedPointer<AccountingEntrySet>> addFilePath(
            int year,
            int month,
            const QString &filePath,
            const QString &filePathDisplay,
            bool copy = true);
    virtual QDate guessDate(const QString &filePath) const;

    virtual QStringList fileFilters() const;
    virtual QString account() const = 0;
    virtual QString accountFees() const = 0;
    virtual bool invoicesForFees() const = 0;
    virtual QList<QVariantList> loadValues(
            const QString &absFileName) const = 0;

    QList<QSharedPointer<AccountingEntrySet>> entrySetsFromFilePath(
            const QString &filePath) const;


protected:
    QSharedPointer<AccountingEntrySet> _entrySetFromVariants(
            const QVariantList &variants,
            const QString &filePath) const;
    QString _idEntrySet(const int idRow, const QDate &date) const;
};

class EntryParserBankException : public QException
{
public:
    void raise() const override;
    EntryParserBankException *clone() const override;

    QString error() const;
    void setError(const QString &error);

private:
    QString m_error;
};

#endif // ENTRYPARSERBANK_H
