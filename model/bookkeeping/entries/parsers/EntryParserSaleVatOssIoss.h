#ifndef ENTRYPARSERSALEVATOSSIOSS_H
#define ENTRYPARSERSALEVATOSSIOSS_H

#include "EntryParserOrders.h"

class EntryParserSaleVatOssIoss : public EntryParserOrders
{
public:
    EntryParserSaleVatOssIoss();
    AccountingEntries entries(int year) const;
    Type typeOfEntries() const;
    QString name() const;
    QString journal() const;
    void recordTransactions(const Shipment *shipmentOrRefund);
    void clearTransactions();


private:
    QMap<int, QMultiMap<QDateTime, const Shipment *>> m_shipmentAndRefunds;
};

#endif // ENTRYPARSERSALEVATOSSIOSS_H
