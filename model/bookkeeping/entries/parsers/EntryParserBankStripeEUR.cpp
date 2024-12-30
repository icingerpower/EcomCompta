#include "EntryParserBankStripeEUR.h"

//----------------------------------------------------------
EntryParserBankStripeEUR::EntryParserBankStripeEUR()
    : EntryParserBankStripe()
{
}
//----------------------------------------------------------
EntryParserBankStripeEUR::~EntryParserBankStripeEUR()
{
}
//----------------------------------------------------------
QString EntryParserBankStripeEUR::currency() const
{
    return "EUR";
}
//----------------------------------------------------------
QString EntryParserBankStripeEUR::account() const
{
    return "512400";
}
//----------------------------------------------------------
QString EntryParserBankStripeEUR::accountFees() const
{
    return "627400";
}
//----------------------------------------------------------


