#include "AbstractEntryParser.h"

//----------------------------------------------------------
AbstractEntryParser::AbstractEntryParser()
{
}
//----------------------------------------------------------
AbstractEntryParser::~AbstractEntryParser()
{

}
//----------------------------------------------------------
AccountingEntries AbstractEntryParser::entries(
        QList<int> years) const
{
    AccountingEntries allEntries;
    for (auto year : years) {
        auto yearEntries = entries(year);
        allEntries[year] = yearEntries[year];
    }
    return allEntries;
}
//----------------------------------------------------------
