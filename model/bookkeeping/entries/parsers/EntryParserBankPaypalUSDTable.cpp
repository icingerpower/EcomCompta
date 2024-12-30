#include "EntryParserBankPaypalUSDTable.h"
#include "EntryParserBankPaypalUSD.h"


//----------------------------------------------------------
EntryParserBankPaypalUSDTable::EntryParserBankPaypalUSDTable(
        QObject *parent) : EntryParserBankTable(parent)
{
    m_entryParserBank = new EntryParserBankPaypalUSD();
}
//----------------------------------------------------------
EntryParserBankPaypalUSDTable::~EntryParserBankPaypalUSDTable()
{
    delete m_entryParserBank;
}
//----------------------------------------------------------
EntryParserBank *EntryParserBankPaypalUSDTable::bankParser() const
{
    return m_entryParserBank;
}
//----------------------------------------------------------
EntryParserBankTable *EntryParserBankPaypalUSDTable::copy() const
{
    return new EntryParserBankPaypalUSDTable();
}
//----------------------------------------------------------
