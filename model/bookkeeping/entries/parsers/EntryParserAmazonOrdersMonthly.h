#ifndef ENTRYPARSERAMAZONORDERSMONTHLY_H
#define ENTRYPARSERAMAZONORDERSMONTHLY_H

#include "EntryParserOrders.h"

class Shipment;

class EntryParserAmazonOrdersMonthly
        : public EntryParserOrders
{
public:
    static QString JOURNAL;
    static QString NAME;
    EntryParserAmazonOrdersMonthly();
    ~EntryParserAmazonOrdersMonthly() override;
    void clear();
    void recordTransactions(const Shipment *shipmentOrRefund) override;
    void clearTransactions() override;
    AccountingEntries entries(int year) const override;
    Type typeOfEntries() const override;
    QString name() const override;
    QString journal() const override;

protected:
struct PriceInfos{
    double untaxed;
    const Shipment *shipment;
    double taxes;
};
    ///  year       month    compteclient   regime        country       type               vat rate
    QMap<int, QMap<int, QMap<QString, QMap<QString, QMap<QString, QMap<QString, QMultiMap<QString, PriceInfos>>>>>>> m_shipments;

};

#endif // ENTRYPARSERAMAZONORDERSMONTHLY_H

