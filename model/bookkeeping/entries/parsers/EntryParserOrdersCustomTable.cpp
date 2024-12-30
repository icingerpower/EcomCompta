#include "EntryParserOrdersCustomTable.h"
#include "EntryParserOrdersCustom.h"

//----------------------------------------------------------
EntryParserOrdersCustomTable::EntryParserOrdersCustomTable(
        QObject *object) : EntryParserOrdersTable(object)
{
    m_entryParser = new EntryParserOrdersCustom();
}
//----------------------------------------------------------
EntryParserOrdersCustomTable::~EntryParserOrdersCustomTable()
{
    delete m_entryParser;
}
//----------------------------------------------------------
QString EntryParserOrdersCustomTable::journal() const
{
    return QObject::tr("VT NN AMAZON");
}
//----------------------------------------------------------
void EntryParserOrdersCustomTable::fillEntries(
        AccountingEntries &entries, int year)
{
    entries = m_entryParser->entries(year);
}
//----------------------------------------------------------
EntryParserOrders *EntryParserOrdersCustomTable::saleParser() const
{
    return m_entryParser;
}
//----------------------------------------------------------
