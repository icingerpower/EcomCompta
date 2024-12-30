#ifndef ORDERIMPORTERFNAC_H
#define ORDERIMPORTERFNAC_H

#include "AbstractOrderImporter.h"

class OrderImporterFnac : public AbstractOrderImporter
{
public:
    static const QString NAME;
    static const QString REPORT_SALES_MERCHENT;
    static const QString REPORT_PAYMENTS;
    static const QString SUB_CHANNEL;
    OrderImporterFnac();
    ~OrderImporterFnac() override;
    QString name() const override;
    QString invoicePrefix() const override;
    QString uniqueId() const override;
    QList<ReportType> reportTypes() const override; // For amazon, vat report, payment report, transactions report…
    QSharedPointer<OrdersMapping> loadReport(
            const QString &reportTypeName,
            const QString &fileName,
            int maxYear) const override;
    QList<QStringList> reportForOrderComplete(const Order *order) const override;

private:
    QSharedPointer<OrdersMapping> _loadReportSalesMerchent(
            const QString &fileName,
            int maxYear) const;
    QSharedPointer<OrdersMapping> _loadReportPayments(
            const QString &fileName,
            int maxYear) const;
};

#endif // ORDERIMPORTERFNAC_H
