#ifndef ENTRYPARSERFNACMONTHLY_H
#define ENTRYPARSERFNACMONTHLY_H

#include <QtCore/qmap.h>

#include "EntryParserMarketplaceMonthly.h"

class EntryParserFnacMonthly
        : public EntryParserMarketplaceMonthly
{
public:
    //static QStringList FEES;
    static QString NAME;
    static QString JOURNAL;
    EntryParserFnacMonthly();
    ~EntryParserFnacMonthly() override;
    QString name() const override;
    QString journal() const override;
    QString nameSupplierCustomer() const override;
    void recordTransactions(const Shipment *shipmentOrRefund) override;
    void clearTransactions() override;
    std::function<bool(const Shipment *)> acceptShipmentOrRefund() const override;
    QMap<QString, QMultiMap<QString, BalanceAmount>> balanceAmounts(
            int year, int month) const override;

protected:
    QMap<int, QMap<int, double>> m_totalConvertedByYearMonth;
};

#endif // ENTRYPARSERFNACMONTHLY_H
