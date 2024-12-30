#include "model/bookkeeping/entries/parsers/EntryParserCdiscountPayment.h"

#include "EntryParserCdiscountPaymentTable.h"


//----------------------------------------------------------
EntryParserCdiscountPaymentTable::EntryParserCdiscountPaymentTable(
        QObject *object)
    : EntryParserOrdersTable(object)
{
    m_entryParser
            = new EntryParserCdiscountPayment();
}
//----------------------------------------------------------
EntryParserCdiscountPaymentTable::~EntryParserCdiscountPaymentTable()
{
    delete m_entryParser;
}
//----------------------------------------------------------
QString EntryParserCdiscountPaymentTable::journal() const
{
    return EntryParserCdiscountPayment::JOURNAL;
}
//----------------------------------------------------------
void EntryParserCdiscountPaymentTable::fillEntries(
        AccountingEntries &entries, int year)
{
    entries = m_entryParser->entries(year);
}
//----------------------------------------------------------
EntryParserOrders *EntryParserCdiscountPaymentTable::saleParser() const
{
    return m_entryParser;
}
//----------------------------------------------------------
EntryParserCdiscountPayment *EntryParserCdiscountPaymentTable::entryParserCdiscountPayment() const
{
    return m_entryParser;
}
//----------------------------------------------------------
