#include "EntryParserBankStripeUSD.h"

//----------------------------------------------------------
EntryParserBankStripeUSD::EntryParserBankStripeUSD()
    : EntryParserBankStripe()
{
}
//----------------------------------------------------------
EntryParserBankStripeUSD::~EntryParserBankStripeUSD()
{
}
//----------------------------------------------------------
QString EntryParserBankStripeUSD::currency() const
{
    return "USD";
}
//----------------------------------------------------------
QString EntryParserBankStripeUSD::account() const
{
    return "512401";
}
//----------------------------------------------------------
QString EntryParserBankStripeUSD::accountFees() const
{
    return "627400";
}
//----------------------------------------------------------

