#include "EntryParserBankWiseEUR.h"


//----------------------------------------------------------
EntryParserBankWiseEUR::EntryParserBankWiseEUR()
    : EntryParserBankWise()
{
}
//----------------------------------------------------------
EntryParserBankWiseEUR::~EntryParserBankWiseEUR()
{
}
//----------------------------------------------------------
QString EntryParserBankWiseEUR::currency() const
{
    return "EUR";
}
//----------------------------------------------------------
QString EntryParserBankWiseEUR::account() const
{
    return "512600"; //TODO add journal name default. Then, I should give possibility to edit this
}
//----------------------------------------------------------
QString EntryParserBankWiseEUR::accountFees() const
{
    return "627500";
}
//----------------------------------------------------------
