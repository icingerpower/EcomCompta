#include "EntryParserCdiscountMonthlyTable.h"
#include "EntryParserCdiscountMonthly.h"

//----------------------------------------------------------
EntryParserCdiscountMonthlyTable::EntryParserCdiscountMonthlyTable(
        QObject *object)
    : EntryParserOrdersTable(object)
{
    m_entryParserCdiscountMonthly
            = new EntryParserCdiscountMonthly();
}
//----------------------------------------------------------
EntryParserCdiscountMonthlyTable::~EntryParserCdiscountMonthlyTable()
{
    delete m_entryParserCdiscountMonthly;
}
//----------------------------------------------------------
QString EntryParserCdiscountMonthlyTable::journal() const
{
    return EntryParserCdiscountMonthly::JOURNAL;
}
//----------------------------------------------------------
void EntryParserCdiscountMonthlyTable::fillEntries(
        AccountingEntries &entries, int year)
{
    entries = m_entryParserCdiscountMonthly->entries(year);
}
//----------------------------------------------------------
EntryParserOrders *EntryParserCdiscountMonthlyTable::saleParser() const
{
    return m_entryParserCdiscountMonthly;
}
//----------------------------------------------------------
EntryParserCdiscountMonthly *EntryParserCdiscountMonthlyTable::entryParserCdiscountMonthly() const
{
    return m_entryParserCdiscountMonthly;
}
//----------------------------------------------------------
bool EntryParserCdiscountMonthlyTable::displays() const
{
    return false;
}
//----------------------------------------------------------
