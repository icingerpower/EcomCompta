#include "EntryParserBankQontoTable.h"

#include "EntryParserBankQonto.h"

//----------------------------------------------------------
EntryParserBankQontoTable::EntryParserBankQontoTable(
        QObject *parent) : EntryParserBankTable(parent)
{
    m_entryParserBank = new EntryParserBankQonto();
}
//----------------------------------------------------------
EntryParserBankQontoTable::~EntryParserBankQontoTable()
{
    delete m_entryParserBank;
}
//----------------------------------------------------------
EntryParserBank *EntryParserBankQontoTable::bankParser() const
{
    return m_entryParserBank;
}
//----------------------------------------------------------
EntryParserBankTable *EntryParserBankQontoTable::copy() const
{
    return new EntryParserBankQontoTable();
}
//----------------------------------------------------------
