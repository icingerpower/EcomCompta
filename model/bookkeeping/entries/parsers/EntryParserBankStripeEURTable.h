#ifndef ENTRYPARSERBANKSTRIPEEURTABLE_H
#define ENTRYPARSERBANKSTRIPEEURTABLE_H

#include "EntryParserBankTable.h"

class EntryParserBankStripeEURTable : public EntryParserBankTable
{
public:
    EntryParserBankStripeEURTable(QObject *parent = nullptr);
    ~EntryParserBankStripeEURTable() override;

    EntryParserBank *bankParser() const override;
    EntryParserBankTable *copy() const override;

private:
    EntryParserBank *m_entryParserBank;
};

#endif // ENTRYPARSERBANKSTRIPEEURTABLE_H
