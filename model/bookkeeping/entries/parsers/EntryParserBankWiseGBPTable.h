#ifndef ENTRYPARSERBANKWISEGBPTABLE_H
#define ENTRYPARSERBANKWISEGBPTABLE_H

#include "EntryParserBankTable.h"

class EntryParserBankWiseGBPTable : public EntryParserBankTable
{
    Q_OBJECT
public:
    EntryParserBankWiseGBPTable(QObject *parent = nullptr);
    ~EntryParserBankWiseGBPTable() override;

    EntryParserBank *bankParser() const override;
    EntryParserBankTable *copy() const override;

private:
    EntryParserBank *m_entryParserBank;
};

#endif // ENTRYPARSERBANKWISEGBPTABLE_H
