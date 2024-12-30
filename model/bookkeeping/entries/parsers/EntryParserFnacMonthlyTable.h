#ifndef ENTRYPARSERFNACMONTHLYTABLE_H
#define ENTRYPARSERFNACMONTHLYTABLE_H

#include "model/bookkeeping/entries/parsers/EntryParserOrdersTable.h"

class EntryParserFnacMonthly;

class EntryParserFnacMonthlyTable
        : public EntryParserOrdersTable
{
public:
    EntryParserFnacMonthlyTable(QObject *object = nullptr);
    ~EntryParserFnacMonthlyTable() override;
    QString journal() const override;
    void fillEntries(AccountingEntries &entries, int year) override;

    EntryParserOrders *saleParser() const override;
    EntryParserFnacMonthly *entryParserFnacMonthly() const;
    bool displays() const override;

private:
    EntryParserFnacMonthly *m_entryParserFnacMonthly;
};

#endif // ENTRYPARSERFNACMONTHLYTABLE_H
