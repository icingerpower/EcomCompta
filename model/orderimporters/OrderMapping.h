#ifndef ORDERMAPPING_H
#define ORDERMAPPING_H

#include <QtCore/qsharedpointer.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qmap.h>
#include <QtCore/qhash.h>

#include "Order.h"
#include "Refund.h"
struct OrdersMapping {
    QMap<int, QMultiMap<QDateTime, QSharedPointer<Order>>> orderByDate;
    QMap<int, QMultiMap<QDateTime, QSharedPointer<Shipment>>> shipmentByDate;
    QMap<int, QMultiMap<QDateTime, QSharedPointer<Refund>>> refundByDate;
    QHash<QString, QSharedPointer<Order>> orderById;
    QHash<QString, QSharedPointer<Shipment>> shipmentById;
    QHash<QString, QSharedPointer<Refund>> refundById;

    /// year / subchannel / date / order / amount
    QMap<int, QHash<QString, QMap<QDateTime, QHash<QString, double>>>> orderByDateUncomplete; /// When no information about articles refunded

    /// year / reportType / date / order / amount
    QMap<int, QHash<QString, QMap<QDateTime, QHash<QString, double>>>> refundByDateUncomplete; /// When no information about articles refunded
    QMultiHash<QString, QSharedPointer<Refund>> refundByOrderId;
    QMap<int, QHash<QString, QMap<QDate, int>>> ordersQuantityByDate; /// By year, subchannel and date
    /// Year   settlement-id      date                fee-name   amount
    QMap<int, QHash<QString, QMap<QDateTime, QMultiMap<QString, double>>>> nonOrderFees;
    QDateTime minDate;
    QDateTime maxDate;
    /// year            datetime         countryfrom    countryto      sku      units
    QMap<int, QMap<QDate, QHash<QString, QHash<QString, QMultiHash<QString, int>>>>> inventoryDeported;

    OrdersMapping();
    int nOrders() const;
    int nShipments() const;

    void removeRefund(QSharedPointer<Refund> refund);
    void addYears(int nYears = 10);
    void removeEmptyYears();
    void uniteFromSameKindOfReport(const OrdersMapping &ordersMapping); //TODO unit + MultiMap
    void unite(const OrdersMapping &ordersMapping); //TODO unit + MultiMap
    void uniteMinMaxDate(const OrdersMapping &ordersMapping); //TODO unit + MultiMap
    void uniteShipments(const OrdersMapping &ordersMapping); //TODO unit + MultiMap
    void uniteOrders(const OrdersMapping &ordersMapping); //TODO unit + MultiMap
    void uniteRefunds(const OrdersMapping &ordersMapping); //TODO unit + MultiMap
    void uniteNonOrderFees(const OrdersMapping &ordersMapping); //TODO unit + MultiMap
    void uniteInventoryDeported(const OrdersMapping &ordersMapping); //TODO unit + MultiMap
    void uniteUncompleteRefund(const OrdersMapping &ordersMapping); //TODO unit + MultiMap
    void uniteUncompleteOrders(const OrdersMapping &ordersMapping); //TODO unit + MultiMap
    void computeOrderNumbers(); //TODO unit + MultiMap
    void createRefundsFromUncomplete();
    void initOrdersUncomplete(const QDateTime &dateTime, const QString &subChannel);
    void initRefundsUncomplete(const QDateTime &dateTime, const QString &reportType);

};


#endif // ORDERMAPPING_H
