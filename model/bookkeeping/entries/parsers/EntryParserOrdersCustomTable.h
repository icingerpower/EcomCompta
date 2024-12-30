#ifndef ENTRYPARSERORDERSCUSTOMTABLE_H
#define ENTRYPARSERORDERSCUSTOMTABLE_H

#include "model/bookkeeping/entries/parsers/EntryParserOrdersTable.h"

class EntryParserOrdersCustom;

class EntryParserOrdersCustomTable
        : public EntryParserOrdersTable
{
    Q_OBJECT
public:
    EntryParserOrdersCustomTable(QObject *object = nullptr);
    ~EntryParserOrdersCustomTable() override;
    QString journal() const override;
    void fillEntries(AccountingEntries &entries, int year) override;

    EntryParserOrders *saleParser() const override;

private:
    EntryParserOrdersCustom *m_entryParser;
};


#endif // ENTRYPARSERORDERSCUSTOMTABLE_H
