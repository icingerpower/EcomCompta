#ifndef ENTRYPARSERBANKWISEEURTABLE_H
#define ENTRYPARSERBANKWISEEURTABLE_H

#include "EntryParserBankTable.h"

class EntryParserBankWiseEURTable : public EntryParserBankTable
{
    Q_OBJECT
public:
    EntryParserBankWiseEURTable(QObject *parent = nullptr);
    ~EntryParserBankWiseEURTable() override;

    EntryParserBank *bankParser() const override;
    EntryParserBankTable *copy() const override;

private:
    EntryParserBank *m_entryParserBank;
};

#endif // ENTRYPARSERBANKWISEEURTABLE_H
