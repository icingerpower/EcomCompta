#include "EntryParserBankStripeUSDTable.h"
#include "EntryParserBankStripeUSD.h"

//----------------------------------------------------------
EntryParserBankStripeUSDTable::EntryParserBankStripeUSDTable(
        QObject *parent) : EntryParserBankTable(parent)
{
    m_entryParserBank = new EntryParserBankStripeUSD();
}
//----------------------------------------------------------
EntryParserBankStripeUSDTable::~EntryParserBankStripeUSDTable()
{
    delete m_entryParserBank;
}
//----------------------------------------------------------
EntryParserBank *EntryParserBankStripeUSDTable::bankParser() const
{
    return m_entryParserBank;
}
//----------------------------------------------------------
EntryParserBankTable *EntryParserBankStripeUSDTable::copy() const
{
    return new EntryParserBankStripeUSDTable();
}
//----------------------------------------------------------
