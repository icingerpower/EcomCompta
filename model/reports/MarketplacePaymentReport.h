#ifndef MARKETPLACEPAYMENTREPORT_H
#define MARKETPLACEPAYMENTREPORT_H

#include <QtCore/qstring.h>
#include <QtCore/qdatetime.h>

#include "AbstractReportGenerator.h"

class Shipment;

class MarketplacePaymentReport : public AbstractReportGenerator
{
public:
    MarketplacePaymentReport();
    void addMarketplaceName(const QString &name);
    void addPaymentDate(const QDateTime &start, QDateTime &end);
    void addPaymentId(const QString &paymentId);
    void startTableAmount();
    void addAmount(
            const QString &account,
            const QString &name,
            double amount);
    void endTableAmount();
    void addAmountWithDetails(
            const QString &account,
            const QString &name,
            double amount,
            QStringList details,
            QList<double> amounts);

};

#endif // MARKETPLACEPAYMENTREPORT_H
