#ifndef ORDERIMPORTERCUSTOM_H
#define ORDERIMPORTERCUSTOM_H

#include "AbstractOrderImporter.h"

class OrderImporterCustom : public AbstractOrderImporter
{
public:
    static QString REPORT_TYPE;
    OrderImporterCustom(const QString &paramsId);
    QString name() const override;
    QString invoicePrefix() const override;
    QString uniqueId() const override;
    QList<ReportType> reportTypes() const override; // For amazon, vat report, payment report, transactions report…
    QSharedPointer<OrdersMapping> loadReport(
            const QString &reportTypeName,
            const QString &fileName,
            int maxYear) const override;
    QList<QStringList> reportForOrderComplete(const Order *order) const override; // For amazon, vat report, payment report, transactions report…

protected:
    QString m_paramsId;
};

#endif // ORDERIMPORTERCUSTOM_H
