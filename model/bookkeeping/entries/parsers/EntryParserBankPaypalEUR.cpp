#include "EntryParserBankPaypalEUR.h"

//----------------------------------------------------------
EntryParserBankPaypalEUR::EntryParserBankPaypalEUR()
    : EntryParserBankPaypal()
{
}
//----------------------------------------------------------
EntryParserBankPaypalEUR::~EntryParserBankPaypalEUR()
{
}
//----------------------------------------------------------
QString EntryParserBankPaypalEUR::currency() const
{
    return "EUR";
}
//----------------------------------------------------------
QString EntryParserBankPaypalEUR::account() const
{
    return "467000";

}
//----------------------------------------------------------
QString EntryParserBankPaypalEUR::accountFees() const
{
    return "627100";
}
//----------------------------------------------------------
