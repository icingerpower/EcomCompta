#ifndef ENTRYPARSERSERVICEORDERS_H
#define ENTRYPARSERSERVICEORDERS_H

#include "EntryParserOrders.h"

class EntryParserServiceOrders : public EntryParserOrders
{
public:
    static const QString JOURNAL;
    EntryParserServiceOrders();
    AccountingEntries entries(int year) const;
    Type typeOfEntries() const;
    QString name() const;
    QString journal() const;
    void recordTransactions(const Shipment *shipmentOrRefund);
    void clearTransactions();

private:
    QMap<int, QMultiMap<QDateTime, const Shipment *>> m_shipmentAndRefunds;
};

#endif // ENTRYPARSERSERVICEORDERS_H
