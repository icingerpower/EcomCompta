#include "EntryParserBankWiseUSD.h"


//----------------------------------------------------------
EntryParserBankWiseUSD::EntryParserBankWiseUSD()
    : EntryParserBankWise()
{
}
//----------------------------------------------------------
EntryParserBankWiseUSD::~EntryParserBankWiseUSD()
{

}
//----------------------------------------------------------
QString EntryParserBankWiseUSD::currency() const
{
    return "USD";
}
//----------------------------------------------------------
QString EntryParserBankWiseUSD::account() const
{
    return "512700"; //TODO add journal name default. Then, I should give possibility to edit this
}
//----------------------------------------------------------
QString EntryParserBankWiseUSD::accountFees() const
{
    return "627700";
}
//----------------------------------------------------------
