#ifndef ENTRYPARSERMARKETPLACEMONTHLY_H
#define ENTRYPARSERMARKETPLACEMONTHLY_H

#include "EntryParserOrders.h"

class EntryParserMarketplaceMonthly
        : public EntryParserOrders
{
public:
    EntryParserMarketplaceMonthly();
    ~EntryParserMarketplaceMonthly();
    virtual std::function<bool(const Shipment *)> acceptShipmentOrRefund() const = 0;

    struct BalanceAmount{
        QString reportFileName;
        QString reportId;
        QString title;
        QString orderId;
        QDate date;
        QString row;
        double amount;
    };

    virtual QMap<QString, QMultiMap<QString, BalanceAmount>> balanceAmounts(
            int year, int month) const = 0;
    virtual QString nameSupplierCustomer() const = 0;
    Type typeOfEntries() const override;
    void recordTransactions(const Shipment *shipmentOrRefund) override;
    void clearTransactions() override;
    AccountingEntries entries(int year) const override;

protected:
    struct PriceInfos{
        double untaxed;
        const Shipment *shipment;
        double taxes;
    };
    ///  year       month    compteclient   regime        country       type               vat rate
    QMap<int, QMap<int, QMap<QString, QMap<QString, QMap<QString, QMap<QString, QMultiMap<QString, PriceInfos>>>>>>> m_shipments;
};

#endif // ENTRYPARSERMARKETPLACEMONTHLY_H
