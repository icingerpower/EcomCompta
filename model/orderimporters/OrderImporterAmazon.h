#ifndef ORDERIMPORTERAMAZON_H
#define ORDERIMPORTERAMAZON_H

#include <QtCore/qdatetime.h>
#include <QtCore/qsharedpointer.h>

#include "AbstractOrderImporter.h"
//#include "model/utils/CsvReader.h"
#include "../common/utils/CsvReader.h"

class OrderImporterAmazon : public AbstractOrderImporter
{
public:
    static const QString NAME;
    static const QString REPORT_ORDERS_FBM;
    static const QString REPORT_ORDERS_FBM_SHORT;
    static const QString REPORT_ORDERS_PAYMENTS;
    static const QString REPORT_ORDERS_INVOICING;
    static const QString REPORT_ORDERS_INVOICING_SHORT;
    OrderImporterAmazon();
    ~OrderImporterAmazon() override;
    QString name() const override;
    QString invoicePrefix() const override;
    QString uniqueId() const override;
    QList<ReportType> reportTypes() const override; // For amazon, vat report, payment report, transactions report…
    QSharedPointer<OrdersMapping> loadReport(
            const QString &reportTypeName,
            const QString &fileName,
            int maxYear) const override;
    QList<QStringList> reportForOrderComplete(const Order *order) const override;

    static QDateTime dateTimeFromString(const QString &string);
    static CsvReader createAmazonReader(const QString &fileName);


private:
    Address _addressFromSubchannel(
            const QString &subchannel, const QString &fbaCenterCode) const;
    QSharedPointer<OrdersMapping> _loadReportOrdersFbm(
            const QString &fileName,
            int maxYear) const;
    QSharedPointer<OrdersMapping> _loadReportPayments(
            const QString &fileName,
            int maxYear) const;
    QSharedPointer<OrdersMapping> _loadReportInvoicing(
            const QString &fileName,
            int maxYear) const;
};

#endif // ORDERIMPORTERAMAZON_H
