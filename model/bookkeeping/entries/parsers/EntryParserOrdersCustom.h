#ifndef ENTRYPARSERORDERSCUSTOM_H
#define ENTRYPARSERORDERSCUSTOM_H

#include "EntryParserOrders.h"

class EntryParserOrdersCustom
        : public EntryParserOrders
{
public:
    EntryParserOrdersCustom();
    void clearTransactions() override;
    void recordTransactions(const Shipment *shipmentOrRefund) override;
    AccountingEntries entries(int year) const override;
    Type typeOfEntries() const override;
    QString name() const override;
    QString journal() const override;

private:
    QMap<int, QMultiMap<QDateTime, const Shipment *>> m_shipmentAndRefunds;
};

#endif // ENTRYPARSERORDERSCUSTOM_H
