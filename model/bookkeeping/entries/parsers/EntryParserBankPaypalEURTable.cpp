#include "EntryParserBankPaypalEURTable.h"
#include "EntryParserBankPaypalEUR.h"

//----------------------------------------------------------
EntryParserBankPaypalEURTable::EntryParserBankPaypalEURTable(
        QObject *parent) : EntryParserBankTable(parent)
{
    m_entryParserBank = new EntryParserBankPaypalEUR();
}
//----------------------------------------------------------
EntryParserBankPaypalEURTable::~EntryParserBankPaypalEURTable()
{
    delete m_entryParserBank;
}
//----------------------------------------------------------
EntryParserBank *EntryParserBankPaypalEURTable::bankParser() const
{
    return m_entryParserBank;
}
//----------------------------------------------------------
EntryParserBankTable *EntryParserBankPaypalEURTable::copy() const
{
    return new EntryParserBankPaypalEURTable();
}
//----------------------------------------------------------
