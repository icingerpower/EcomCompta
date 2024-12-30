#ifndef ENTRYPARSERORDERS_H
#define ENTRYPARSERORDERS_H

#include "AbstractEntryParser.h"

class Shipment;

class EntryParserOrders
        : public AbstractEntryParser
{
public:
    EntryParserOrders();
    ~EntryParserOrders() override;
    virtual void recordTransactions(const Shipment *shipmentOrRefund) = 0;
    virtual void clearTransactions() = 0;
};

#endif // ENTRYPARSERORDERS_H
