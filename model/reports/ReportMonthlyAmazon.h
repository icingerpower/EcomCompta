#ifndef REPORTMONTHLYAMAZON_H
#define REPORTMONTHLYAMAZON_H

#include <QtCore/qstring.h>
#include <QtCore/qdatetime.h>

#include "AbstractReportGenerator.h"

class Shipment;

struct TablePriceTotal{
    double untaxed;
    double taxes;
    TablePriceTotal();
};

class ReportMonthlyAmazon : public AbstractReportGenerator
{
public:
    ReportMonthlyAmazon();
    QStringList addTable();
    QStringList addTableRow(const Shipment *shipment, double untaxed, double taxes);
    QStringList addTableTotal(double untaxed, double taxes);
    void addTableMonthlyTotal(int monthMax,
            const QMap<QString, TablePriceTotal> &priceMonthly);

    void addTableNonSale();
    void addTableNonSaleRow(
            const QString &report,
            const QString &reportId,
            const QString &title,
            const QString &orderId,
            const QDate &date,
            const QString &row,
            double amount);
    void addTableNonSaleTotal(
            double amount);

protected:
    struct ColInfo {
        QString name;
        QString (*getValue)(const Shipment *shipment);
    };
    QList<ColInfo> colInfos() const;
    QString m_title;
};

#endif // REPORTMONTHLYAMAZON_H
