#ifndef ENTRYPARSERBANKPAYPALUSDTABLE_H
#define ENTRYPARSERBANKPAYPALUSDTABLE_H


#include "EntryParserBankTable.h"

class EntryParserBankPaypalUSDTable : public EntryParserBankTable
{
public:
    EntryParserBankPaypalUSDTable(QObject *parent = nullptr);
    ~EntryParserBankPaypalUSDTable() override;

    EntryParserBank *bankParser() const override;
    EntryParserBankTable *copy() const override;

private:
    EntryParserBank *m_entryParserBank;
};



#endif // ENTRYPARSERBANKPAYPALUSDTABLE_H
