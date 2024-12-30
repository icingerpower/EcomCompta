#include "EntryParserBankWiseEURTable.h"
#include "EntryParserBankWiseEUR.h"

//----------------------------------------------------------
EntryParserBankWiseEURTable::EntryParserBankWiseEURTable(QObject *parent)
    : EntryParserBankTable(parent)
{
    m_entryParserBank = new EntryParserBankWiseEUR();
}
//----------------------------------------------------------
EntryParserBankWiseEURTable::~EntryParserBankWiseEURTable()
{
    delete m_entryParserBank;
}
//----------------------------------------------------------
EntryParserBank *EntryParserBankWiseEURTable::bankParser() const
{
    return m_entryParserBank;
}
//----------------------------------------------------------
EntryParserBankTable *EntryParserBankWiseEURTable::copy() const
{
    return new EntryParserBankWiseEURTable();
}
//----------------------------------------------------------
