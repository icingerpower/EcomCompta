#include "EntryParserServiceOrders.h"

#include "EntryParserServiceOrdersTable.h"

//----------------------------------------------------------
EntryParserServiceOrdersTable::EntryParserServiceOrdersTable(
        QObject *object) : EntryParserOrdersTable(object)

{
    m_entryParser = new EntryParserServiceOrders();
}
//----------------------------------------------------------
EntryParserServiceOrdersTable::~EntryParserServiceOrdersTable()
{
    delete m_entryParser;
}
//----------------------------------------------------------
QString EntryParserServiceOrdersTable::journal() const
{
    return EntryParserServiceOrders::JOURNAL;
}
//----------------------------------------------------------
void EntryParserServiceOrdersTable::fillEntries(
        AccountingEntries &entries, int year)
{
    entries = m_entryParser->entries(year);
}
//----------------------------------------------------------
EntryParserOrders *EntryParserServiceOrdersTable::saleParser() const
{
    return m_entryParser;
}
//----------------------------------------------------------
