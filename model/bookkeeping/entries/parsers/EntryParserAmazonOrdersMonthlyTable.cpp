#include "EntryParserAmazonOrdersMonthly.h"
#include "EntryParserAmazonOrdersMonthlyTable.h"

//----------------------------------------------------------
EntryParserAmazonOrdersMonthlyTable::EntryParserAmazonOrdersMonthlyTable(
        QObject *object)
    : EntryParserOrdersTable(object)
{
    m_entryParserAmazonOrdersMonthly
            = new EntryParserAmazonOrdersMonthly();
}
//----------------------------------------------------------
EntryParserAmazonOrdersMonthlyTable::~EntryParserAmazonOrdersMonthlyTable()
{
    delete m_entryParserAmazonOrdersMonthly;
}
//----------------------------------------------------------
QString EntryParserAmazonOrdersMonthlyTable::journal() const
{
    return EntryParserAmazonOrdersMonthly::JOURNAL;
}
//----------------------------------------------------------
void EntryParserAmazonOrdersMonthlyTable::fillEntries(
        AccountingEntries &entries, int year)
{
    entries = m_entryParserAmazonOrdersMonthly->entries(year);
}
//----------------------------------------------------------
EntryParserOrders *EntryParserAmazonOrdersMonthlyTable::saleParser() const
{
    return m_entryParserAmazonOrdersMonthly;
}
//----------------------------------------------------------
EntryParserAmazonOrdersMonthly *EntryParserAmazonOrdersMonthlyTable::entryParserAmazonOrdersMonthly() const
{
    return m_entryParserAmazonOrdersMonthly;
}
//----------------------------------------------------------
bool EntryParserAmazonOrdersMonthlyTable::displays() const
{
    return false;
}
//----------------------------------------------------------
