#ifndef ENTRYPARSERBANKPAYPALEURTABLE_H
#define ENTRYPARSERBANKPAYPALEURTABLE_H

#include "EntryParserBankTable.h"

class EntryParserBankPaypalEURTable : public EntryParserBankTable
{
public:
    EntryParserBankPaypalEURTable(QObject *parent = nullptr);
    ~EntryParserBankPaypalEURTable() override;

    EntryParserBank *bankParser() const override;
    EntryParserBankTable *copy() const override;

private:
    EntryParserBank *m_entryParserBank;
};



#endif // ENTRYPARSERBANKPAYPALEURTABLE_H
