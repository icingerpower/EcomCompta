#ifndef ENTRYPARSERCDISCOUNTMONTHLYTABLE_H
#define ENTRYPARSERCDISCOUNTMONTHLYTABLE_H

#include "model/bookkeeping/entries/parsers/EntryParserOrdersTable.h"

class EntryParserCdiscountMonthly;

class EntryParserCdiscountMonthlyTable
        : public EntryParserOrdersTable
{
public:
    EntryParserCdiscountMonthlyTable(QObject *object = nullptr);
    ~EntryParserCdiscountMonthlyTable() override;
    QString journal() const override;
    void fillEntries(AccountingEntries &entries, int year) override;

    EntryParserOrders *saleParser() const override;
    EntryParserCdiscountMonthly *entryParserCdiscountMonthly() const;
    bool displays() const override;

private:
    EntryParserCdiscountMonthly *m_entryParserCdiscountMonthly;
};

#endif // ENTRYPARSERCDISCOUNTMONTHLYTABLE_H
