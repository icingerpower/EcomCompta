#include "EntryParserStockDeported.h"

#include "EntryParserStockDeportedTable.h"

//----------------------------------------------------------
EntryParserStockDeportedTable::EntryParserStockDeportedTable(QObject *object)
    : AbstractEntryParserTable(object)
{
    m_entryParser = new EntryParserStockDeported;
}
//----------------------------------------------------------
EntryParserStockDeportedTable::~EntryParserStockDeportedTable()
{
    delete m_entryParser;
}
//----------------------------------------------------------
QString EntryParserStockDeportedTable::name() const
{
    return m_entryParser->name();
}
//----------------------------------------------------------
QString EntryParserStockDeportedTable::journal() const
{
    return m_entryParser->journal();
}
//----------------------------------------------------------
void EntryParserStockDeportedTable::fillEntries(
        AccountingEntries &entries, int year)
{
    entries = m_entryParser->entries(year);
}
//----------------------------------------------------------
