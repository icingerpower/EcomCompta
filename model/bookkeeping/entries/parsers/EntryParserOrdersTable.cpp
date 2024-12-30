#include "EntryParserOrdersTable.h"
#include "EntryParserOrders.h"

//----------------------------------------------------------
EntryParserOrdersTable::EntryParserOrdersTable(QObject *parent)
    : AbstractEntryParserTable(parent)
{
}
//----------------------------------------------------------
EntryParserOrdersTable::~EntryParserOrdersTable()
{
}
//----------------------------------------------------------
QString EntryParserOrdersTable::name() const
{
    return saleParser()->name();
}
//----------------------------------------------------------
