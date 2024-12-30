#ifndef ENTRYPARSERCDISCOUNTMONTHLY_H
#define ENTRYPARSERCDISCOUNTMONTHLY_H

#include <QtCore/qmap.h>

#include "EntryParserMarketplaceMonthly.h"

class EntryParserCdiscountMonthly
        : public EntryParserMarketplaceMonthly
{
public:
    //static QStringList FEES;
    static QString NAME;
    static QString JOURNAL;
    EntryParserCdiscountMonthly();
    ~EntryParserCdiscountMonthly() override;
    QString name() const override;
    QString journal() const override;
    QString nameSupplierCustomer() const override;
    void clearTransactions() override;
    void recordTransactions(const Shipment *shipmentOrRefund) override;
    std::function<bool(const Shipment *)> acceptShipmentOrRefund() const override;
    QMap<QString, QMultiMap<QString, BalanceAmount>> balanceAmounts(
            int year, int month) const override;

    static double cdiscountToDouble(QString &string);
protected:
    QMap<int, QMap<int, double>> m_totalConvertedByYearMonth;
};

#endif // ENTRYPARSERCDISCOUNTMONTHLY_H
