#include "EntryParserBankWiseGBPTable.h"
#include "EntryParserBankWiseGBP.h"


//----------------------------------------------------------
EntryParserBankWiseGBPTable::EntryParserBankWiseGBPTable(QObject *parent)
    : EntryParserBankTable(parent)
{
    m_entryParserBank = new EntryParserBankWiseGBP();
}
//----------------------------------------------------------
EntryParserBankWiseGBPTable::~EntryParserBankWiseGBPTable()
{
    delete m_entryParserBank;
}
//----------------------------------------------------------
EntryParserBank *EntryParserBankWiseGBPTable::bankParser() const
{
    return m_entryParserBank;
}
//----------------------------------------------------------
EntryParserBankTable *EntryParserBankWiseGBPTable::copy() const
{
    return new EntryParserBankWiseGBPTable();
}
//----------------------------------------------------------
