#ifndef ENTRYPARSERBANKSTRIPEUSDTABLE_H
#define ENTRYPARSERBANKSTRIPEUSDTABLE_H

#include "EntryParserBankTable.h"

class EntryParserBankStripeUSDTable : public EntryParserBankTable
{
public:
    EntryParserBankStripeUSDTable(QObject *parent = nullptr);
    ~EntryParserBankStripeUSDTable() override;

    EntryParserBank *bankParser() const override;
    EntryParserBankTable *copy() const override;

private:
    EntryParserBank *m_entryParserBank;
};


#endif // ENTRYPARSERBANKSTRIPEUSDTABLE_H
