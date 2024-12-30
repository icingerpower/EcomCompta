#ifndef ENTRYPARSERSTOCKDEPORTED_H
#define ENTRYPARSERSTOCKDEPORTED_H

#include "AbstractEntryParser.h"

class EntryParserStockDeported : public AbstractEntryParser
{
public:
    EntryParserStockDeported();
    AccountingEntries entries(int year) const;
    Type typeOfEntries() const;
    QString name() const;
    QString journal() const;
};

#endif // ENTRYPARSERSTOCKDEPORTED_H
