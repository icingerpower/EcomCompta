#ifndef ENTRYPARSERBANKTABLE_H
#define ENTRYPARSERBANKTABLE_H

#include "model/bookkeeping/entries/parsers/AbstractEntryParserTable.h"
#include "model/bookkeeping/entries/parsers/EntryParserBank.h"

class EntryParserBankTable : public AbstractEntryParserTable
{
    Q_OBJECT
public:
    EntryParserBankTable(QObject *parent = nullptr);

    virtual EntryParserBank *bankParser() const = 0;
    virtual EntryParserBankTable *copy() const = 0;

    QString name() const override;
    void fillEntries(AccountingEntries &entries, int year) override;

    QString journal() const override;
    QStringList fileFilters() const; /// *.csv
    QString fileFiltersDialog() const; /// *.csv
    void addFilePath(int year,
                     int month,
                     const QString &filePath,
                     const QString &filePathDisplay,
                     bool copy = true);
    QDate guessDate(const QString &filePath) const;
};

#endif // ENTRPARSERBANKTABLEABSTRACT_H
