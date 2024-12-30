#ifndef ORDERIMPORTERSERVICESALES_H
#define ORDERIMPORTERSERVICESALES_H


#include "AbstractOrderImporter.h"

class OrderImporterServiceSales : public AbstractOrderImporter
{
public:
    static const QString NAME;
    static const QString FILE_SALES;
    OrderImporterServiceSales();

    QString uniqueId() const override;
    QString name() const override;
    QString invoicePrefix() const override;
    QList<ReportType> reportTypes() const override;
    QList<QStringList> reportForOrderComplete(
            const Order *order) const override;
    QSharedPointer<OrdersMapping> loadReport(
            const QString &reportTypeName,
            const QString &fileName,
            int maxYear) const override;
};

#endif // ORDERIMPORTERSERVICESALES_H
