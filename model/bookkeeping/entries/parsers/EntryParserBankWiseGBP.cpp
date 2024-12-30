#include "EntryParserBankWiseGBP.h"


//----------------------------------------------------------
EntryParserBankWiseGBP::EntryParserBankWiseGBP()
    : EntryParserBankWise()
{
}
//----------------------------------------------------------
EntryParserBankWiseGBP::~EntryParserBankWiseGBP()
{

}
//----------------------------------------------------------
QString EntryParserBankWiseGBP::currency() const
{
    return "GBP";
}
//----------------------------------------------------------
QString EntryParserBankWiseGBP::account() const
{
    return "512800"; //TODO add journal name default. Then, I should give possibility to edit this
}
//----------------------------------------------------------
QString EntryParserBankWiseGBP::accountFees() const
{
    return "627600";
}
//----------------------------------------------------------
