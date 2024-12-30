#include "EntryParserBankStripeEURTable.h"
#include "EntryParserBankStripeEUR.h"

//----------------------------------------------------------
EntryParserBankStripeEURTable::EntryParserBankStripeEURTable(
        QObject *parent) : EntryParserBankTable(parent)
{
    m_entryParserBank = new EntryParserBankStripeEUR();
}
//----------------------------------------------------------
EntryParserBankStripeEURTable::~EntryParserBankStripeEURTable()
{
    delete m_entryParserBank;
}
//----------------------------------------------------------
EntryParserBank *EntryParserBankStripeEURTable::bankParser() const
{
    return m_entryParserBank;
}
//----------------------------------------------------------
EntryParserBankTable *EntryParserBankStripeEURTable::copy() const
{
    return new EntryParserBankStripeEURTable();
}
//----------------------------------------------------------
