#ifndef ENTRYPARSERBANKQONTOTABLE_H
#define ENTRYPARSERBANKQONTOTABLE_H

#include "EntryParserBankTable.h"


class EntryParserBankQontoTable : public EntryParserBankTable
{
    Q_OBJECT
public:
    EntryParserBankQontoTable(QObject *parent = nullptr);
    ~EntryParserBankQontoTable() override;

    EntryParserBank *bankParser() const override;
    EntryParserBankTable *copy() const override;


private:
    EntryParserBank *m_entryParserBank;
};

#endif // ENTRYPARSERBANKQONTOTABLE_H
