#include "EntryParserSaleVatOssIoss.h"

#include "EntryParserSaleVatOssIossTable.h"

//----------------------------------------------------------
EntryParserSaleVatOssIossTable::EntryParserSaleVatOssIossTable(
        QObject *object) : EntryParserOrdersTable(object)
{
    m_entryParser = new EntryParserSaleVatOssIoss();
}
//----------------------------------------------------------
EntryParserSaleVatOssIossTable::~EntryParserSaleVatOssIossTable()
{
    delete m_entryParser;
}
//----------------------------------------------------------
QString EntryParserSaleVatOssIossTable::journal() const
{
    return m_entryParser->journal();
}
//----------------------------------------------------------
void EntryParserSaleVatOssIossTable::fillEntries(
        AccountingEntries &entries, int year)
{
    entries = m_entryParser->entries(year);
}
//----------------------------------------------------------
bool EntryParserSaleVatOssIossTable::displays() const
{
    return false;
}
//----------------------------------------------------------
EntryParserOrders *EntryParserSaleVatOssIossTable::saleParser() const
{
    return m_entryParser;
}
//----------------------------------------------------------
