#ifndef ENTRYPARSERORDERSTABLE_H
#define ENTRYPARSERORDERSTABLE_H

#include "model/bookkeeping/entries/parsers/AbstractEntryParserTable.h"
class EntryParserOrders;

class EntryParserOrdersTable : public AbstractEntryParserTable
{
public:
    EntryParserOrdersTable(QObject *parent = nullptr);
    ~EntryParserOrdersTable() override;

    virtual EntryParserOrders *saleParser() const = 0;
    virtual QString name() const override;
};

#endif // ENTRYPARSERORDERSTABLE_H
