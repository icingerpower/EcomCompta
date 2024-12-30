#include "EntryParserFnacMonthlyTable.h"
#include "EntryParserFnacMonthly.h"

//----------------------------------------------------------
EntryParserFnacMonthlyTable::EntryParserFnacMonthlyTable(
        QObject *object)
    : EntryParserOrdersTable(object)
{
    m_entryParserFnacMonthly
            = new EntryParserFnacMonthly();
}
//----------------------------------------------------------
EntryParserFnacMonthlyTable::~EntryParserFnacMonthlyTable()
{
    delete m_entryParserFnacMonthly;
}
//----------------------------------------------------------
QString EntryParserFnacMonthlyTable::journal() const
{
    return EntryParserFnacMonthly::JOURNAL;
}
//----------------------------------------------------------
void EntryParserFnacMonthlyTable::fillEntries(
        AccountingEntries &entries, int year)
{
    entries = m_entryParserFnacMonthly->entries(year);
}
//----------------------------------------------------------
EntryParserOrders *EntryParserFnacMonthlyTable::saleParser() const
{
    return m_entryParserFnacMonthly;
}
//----------------------------------------------------------
EntryParserFnacMonthly *EntryParserFnacMonthlyTable::entryParserFnacMonthly() const
{
    return m_entryParserFnacMonthly;
}
//----------------------------------------------------------
bool EntryParserFnacMonthlyTable::displays() const
{
    return false;
}
//----------------------------------------------------------
