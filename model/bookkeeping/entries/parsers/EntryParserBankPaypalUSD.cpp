#include "EntryParserBankPaypalUSD.h"

//----------------------------------------------------------
EntryParserBankPaypalUSD::EntryParserBankPaypalUSD()
    : EntryParserBankPaypal()
{
}
//----------------------------------------------------------
EntryParserBankPaypalUSD::~EntryParserBankPaypalUSD()
{
}
//----------------------------------------------------------
QString EntryParserBankPaypalUSD::currency() const
{
    return "USD";
}
//----------------------------------------------------------
QString EntryParserBankPaypalUSD::account() const
{
    return "467001";
}
//----------------------------------------------------------
QString EntryParserBankPaypalUSD::accountFees() const
{
    return "627100";
}
//----------------------------------------------------------
