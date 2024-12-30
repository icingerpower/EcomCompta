#ifndef ENTRYPARSERSTOCKDEPORTEDTABLE_H
#define ENTRYPARSERSTOCKDEPORTEDTABLE_H

#include "AbstractEntryParserTable.h"

class EntryParserStockDeported;

class EntryParserStockDeportedTable : public AbstractEntryParserTable
{
    Q_OBJECT
public:
    EntryParserStockDeportedTable(QObject *object = nullptr);
    ~EntryParserStockDeportedTable() override;
    QString name() const override;
    QString journal() const override;
    void fillEntries(AccountingEntries &entries, int year) override;

private:
    EntryParserStockDeported *m_entryParser;

};

#endif // ENTRYPARSERSTOCKDEPORTEDTABLE_H
