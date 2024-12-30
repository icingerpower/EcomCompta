#include "EntryParserBankWiseUSDTable.h"
#include "EntryParserBankWiseUSD.h"


//----------------------------------------------------------
EntryParserBankWiseUSDTable::EntryParserBankWiseUSDTable(QObject *parent)
    : EntryParserBankTable(parent)
{
    m_entryParserBank = new EntryParserBankWiseUSD();
}
//----------------------------------------------------------
EntryParserBankWiseUSDTable::~EntryParserBankWiseUSDTable()
{
    delete m_entryParserBank;
}
//----------------------------------------------------------
EntryParserBank *EntryParserBankWiseUSDTable::bankParser() const
{
    return m_entryParserBank;
}
//----------------------------------------------------------
EntryParserBankTable *EntryParserBankWiseUSDTable::copy() const
{
    return new EntryParserBankWiseUSDTable();
}
//----------------------------------------------------------
