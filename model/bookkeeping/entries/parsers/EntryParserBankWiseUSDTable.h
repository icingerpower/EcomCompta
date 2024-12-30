#ifndef ENTRYPARSERBANKWISEUSDTABLE_H
#define ENTRYPARSERBANKWISEUSDTABLE_H

#include "EntryParserBankTable.h"

class EntryParserBankWiseUSDTable : public EntryParserBankTable
{
    Q_OBJECT
public:
    EntryParserBankWiseUSDTable(QObject *parent = nullptr);
    ~EntryParserBankWiseUSDTable() override;

    EntryParserBank *bankParser() const override;
    EntryParserBankTable *copy() const override;

private:
    EntryParserBank *m_entryParserBank;
};

#endif // ENTRYPARSERBANKWISEUSDTABLE_H
