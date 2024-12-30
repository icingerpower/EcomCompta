#ifndef LOGVAT_H
#define LOGVAT_H

#include <QDateTime>
#include <QMap>
#include <QString>

class Shipment;

///         year      date           vat-regime   vat-country-to  sale-type      vat-rate      order-id    shipment-id
typedef QMap<int, QMap<QDateTime, QMap<QString, QMap<QString, QMap<QString, QMap<QString, QMap<QString, QMap<QString, QPair<double, double>>>>>>>>> SortedAmounts;

class LogVat
{
public:
    static LogVat *instance();
    void recordAmount(const QString &typeComputing, const Shipment *shipmentOrRefund);
    void recordAmount(const QString &typeComputing, const Shipment *shipmentOrRefund, double amountUntaxed, double amountTaxed);
    void saveLog(const QString &typeComputing, const QString &fileName, int year, int monthFrom, int monthTo);

private:
    QHash<QString, SortedAmounts> m_amounts;
};

#endif // LOGVAT_H
