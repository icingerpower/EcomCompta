#include <QtCore/qdatetime.h>

#include "model/vat/VatRateManager.h"
#include "model/bookkeeping/ManagerSaleTypes.h"
#include "model/orderimporters/ShippingAddressesManager.h"

#include "OrderMapping.h"
#include "Order.h"


//----------------------------------------------------------
//----------------------------------------------------------
OrdersMapping::OrdersMapping()
{
    minDate = QDateTime::currentDateTime();
    maxDate = QDateTime(QDate(2000,1,1));
}
//----------------------------------------------------------qoqmhf
int OrdersMapping::nOrders() const
{
    return orderById.size();
}
//----------------------------------------------------------
int OrdersMapping::nShipments() const
{
    return shipmentById.size();
}
//----------------------------------------------------------
void OrdersMapping::removeRefund(QSharedPointer<Refund> refund)
{
    auto refundDate = refund->getDateTime();
    int year = refundDate.date().year();
    QString refundId = refund->getId();
    refundById.remove(refundId);
    if (refundByOrderId.contains(refund->orderId())) {
        refundByOrderId.remove(refund->orderId());
    }
    auto allRefunds = refundByDate[year].values(refundDate);
    int nRemoved = refundByDate[year].remove(refundDate);
    if (nRemoved > 1) {
        allRefunds.removeOne(refund);
        for (auto value : allRefunds) {
            refundByDate[year].insert(refundDate, value);
        }
    }
}
//----------------------------------------------------------
void OrdersMapping::addYears(int nYears)
{
    int currentYear = QDate::currentDate().year();
    for (int year = currentYear - nYears; year <= currentYear; year++) {
        orderByDate[year]
                = QMap<QDateTime, QSharedPointer<Order>>();
        shipmentByDate[year]
                = QMap<QDateTime, QSharedPointer<Shipment>>();
        ordersQuantityByDate[year]
                = QHash<QString, QMap<QDate, int>>();
        nonOrderFees[year] = QHash<QString, QMap<QDateTime, QMultiMap<QString, double>>>();
        inventoryDeported[year] = QMap<QDate, QHash<QString, QHash<QString, QMultiHash<QString, int>>>>();
    }
}
//----------------------------------------------------------
//----------------------------------------------------------
void OrdersMapping::removeEmptyYears()
{
    auto years = orderByDate.keys();
    for (int year : years) {
        if (orderByDate[year].isEmpty()) {
            orderByDate.remove(year);
        }
    }
    years = shipmentByDate.keys();
    for (int year : years) {
        if (shipmentByDate[year].isEmpty()) {
            shipmentByDate.remove(year);
        }
    }
    years = ordersQuantityByDate.keys();
    for (int year : years) {
        if (ordersQuantityByDate[year].isEmpty()) {
            ordersQuantityByDate.remove(year);
        }
    }
    years = nonOrderFees.keys();
    for (int year : years) {
        if (nonOrderFees[year].isEmpty()) {
            nonOrderFees.remove(year);
        }
    }
    years = inventoryDeported.keys();
    for (int year : years) {
        if (inventoryDeported[year].isEmpty()) {
            inventoryDeported.remove(year);
        }
    }
}
//----------------------------------------------------------
void OrdersMapping::uniteFromSameKindOfReport(
        const OrdersMapping &ordersMapping)
{
    uniteMinMaxDate(ordersMapping);
    uniteOrders(ordersMapping);
    uniteShipments(ordersMapping);
    computeOrderNumbers();
    if (orderById.size() > 0) {
        auto firstOrder = orderById.begin().value();
        if (firstOrder->getDateTime().isValid() || firstOrder->getShipmentCount() > 0) {
            //computeOrderNumbers();
        } else { /// We are from VAT report for which there is no valid dates but no duplicate lines
            //ordersQuantityByDate.unite(ordersMapping.ordersQuantityByDate);
            for (auto itYear = ordersMapping.ordersQuantityByDate.begin();
                 itYear != ordersMapping.ordersQuantityByDate.end();
                 ++itYear) {
                if (!ordersQuantityByDate.contains(itYear.key())) {
                    ordersQuantityByDate[itYear.key()] = itYear.value();
                } else {
                    for (auto itSubchannel = itYear.value().begin();
                         itSubchannel != itYear.value().end();
                         ++itSubchannel) {
                        if (!ordersQuantityByDate[itYear.key()].contains(itSubchannel.key())) {
                            ordersQuantityByDate[itYear.key()][itSubchannel.key()]
                                    = ordersMapping.ordersQuantityByDate[itYear.key()][itSubchannel.key()];
                        } else {
                            for (auto itDate = itSubchannel.value().begin();
                                 itDate != itSubchannel.value().end();
                                 ++itDate) {
                                Q_ASSERT(itDate.key().isValid());
                                if (!ordersQuantityByDate[itYear.key()][itSubchannel.key()].contains(
                                            itDate.key())) {
                                    ordersQuantityByDate[itYear.key()][itSubchannel.key()][itDate.key()]
                                            = itDate.value();
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    uniteRefunds(ordersMapping);
    uniteNonOrderFees(ordersMapping);
    uniteInventoryDeported(ordersMapping);
    uniteUncompleteRefund(ordersMapping);
    uniteUncompleteOrders(ordersMapping);
}
//----------------------------------------------------------
void OrdersMapping::unite(const OrdersMapping &ordersMapping)
{
    uniteMinMaxDate(ordersMapping);
    uniteOrders(ordersMapping);
    uniteShipments(ordersMapping);
    computeOrderNumbers();
    uniteRefunds(ordersMapping);
    uniteNonOrderFees(ordersMapping);
    uniteInventoryDeported(ordersMapping);
    uniteUncompleteRefund(ordersMapping);
    uniteUncompleteOrders(ordersMapping);
}
//----------------------------------------------------------
void OrdersMapping::uniteMinMaxDate(const OrdersMapping &ordersMapping)
{
    minDate = qMin(minDate, ordersMapping.minDate);
    maxDate = qMin(maxDate, ordersMapping.maxDate);
}
//----------------------------------------------------------
void OrdersMapping::uniteShipments(const OrdersMapping &ordersMapping)
{
    for (auto shipment : ordersMapping.shipmentById) {
        if (!shipmentById.contains(shipment->getId())) {
            shipmentById[shipment->getId()] = shipment;
            //QDateTime orderDate = shipment->getDateTime();
           // int year = orderDate.date().year();
            int year = shipment->getDateTime().date().year();
            if (!shipmentByDate.contains(year)) {
                shipmentByDate[year]
                        = QMultiMap<QDateTime, QSharedPointer<Shipment>>();
            }
            shipmentByDate[year].insert(shipment->getDateTime(), shipment);
        } else {
            shipmentById[shipment->getId()]->merge(*shipment.data());
        }
    }
}
//----------------------------------------------------------
void OrdersMapping::uniteOrders(const OrdersMapping &ordersMapping)
{
     for (auto order : ordersMapping.orderById) {
        QDateTime orderDate = order->getDateTime();
        if (!orderById.contains(order->getId())) {
            orderById[order->getId()] = order;
            if(!orderDate.isNull()) {
                int year = orderDate.date().year();
                if (!orderByDate.contains(year)) {
                    orderByDate[year] = QMultiMap<QDateTime, QSharedPointer<Order>>();
                }
                orderByDate[orderDate.date().year()].insert(orderDate, order);
            }
        } else {
            if (orderById[order->getId()].isNull() && !orderDate.isNull()) {
                int year = orderDate.date().year();
                if (!orderByDate.contains(year)) {
                    orderByDate[year] = QMultiMap<QDateTime, QSharedPointer<Order>>();
                }
                orderByDate[orderDate.date().year()].insert(orderDate, order);
            }
            orderById[order->getId()]->merge(*order.data());
        }
    }
}
//----------------------------------------------------------
void OrdersMapping::uniteRefunds(const OrdersMapping &ordersMapping)
{
    bool hasLeft = refundById.contains("amzn1:crow:m6FtH3XbQ7a/wUbCCMh2Gg");
    bool hasRight = ordersMapping.refundById.contains("amzn1:crow:m6FtH3XbQ7a/wUbCCMh2Gg");
    for (auto itRefund = ordersMapping.refundById.begin();
         itRefund != ordersMapping.refundById.end();
         ++itRefund) {
        QString refundId = itRefund.key();
        QString orderId = itRefund.value()->orderId();
        if (!refundById.contains(itRefund.key())) {
            refundById[itRefund.key()] = itRefund.value();
            refundByOrderId.insert(orderId, itRefund.value());
            int year = itRefund.value()->getDateTime().date().year();
            if (!refundByDate.contains(year)) {
                refundByDate[year]
                        = QMap<QDateTime, QSharedPointer<Refund>>();
            }
            refundByDate[year].insert(itRefund.value()->getDateTime(), itRefund.value());
            if (orderById.contains(orderId)) { //Shall we create sinc refunds with order
                itRefund.value()->init(orderById[orderId].data());
            //} else {
                //int TEMP=10;++TEMP;
            }
        } else {
            refundById[itRefund.key()]->merge(*itRefund.value().data());
            if (orderById.contains(orderId)) { //Shall we create sinc refunds with order
                itRefund.value()->init(orderById[orderId].data());
            }
        }
    }
    if (hasLeft || hasRight) {
        Q_ASSERT(refundById.contains("amzn1:crow:m6FtH3XbQ7a/wUbCCMh2Gg"));
    }
}
//----------------------------------------------------------
void OrdersMapping::uniteNonOrderFees(const OrdersMapping &ordersMapping)
{
    for (auto itYear = ordersMapping.nonOrderFees.begin();
         itYear != ordersMapping.nonOrderFees.end();
         ++itYear) {
        if (!nonOrderFees.contains(itYear.key())) {
            nonOrderFees[itYear.key()] = itYear.value();
        } else {
            for (auto itSettlement = itYear.value().begin();
                 itSettlement != itYear.value().end();
                 ++itSettlement) {
                if (!nonOrderFees[itYear.key()].contains(itSettlement.key())) {
                    nonOrderFees[itYear.key()][itSettlement.key()] = itSettlement.value();
                } else {
                    for (auto itDate = itSettlement.value().begin();
                         itDate != itSettlement.value().end();
                         ++itDate) {
                        if (!nonOrderFees[itYear.key()][itSettlement.key()].contains(itDate.key())) {
                            nonOrderFees[itYear.key()][itSettlement.key()][itDate.key()] = itDate.value();
                            ///We should never go there because it is impossible to have same settlement in 2 different files
                            /*
                        } else {
                            for (auto itFeeName = itDate.value().begin();
                                 itFeeName != itDate.value().end();
                                 ++itFeeName) {
                                nonOrderFees[itYear.key()][itSettlement.key()][itDate.key()].insert(itFeeName.key(), itFeeName.value());
                            }
                            //*/
                        }
                    }
                }
            }
        }
    }
}
//----------------------------------------------------------
void OrdersMapping::uniteInventoryDeported(
        const OrdersMapping &ordersMapping)
{
    for (auto itYear = ordersMapping.inventoryDeported.begin();
         itYear != ordersMapping.inventoryDeported.end();
         ++itYear) {
        if (!inventoryDeported.contains(itYear.key()))  {
            inventoryDeported[itYear.key()] = itYear.value();
        } else {
            for (auto itDate = itYear.value().begin();
                 itDate != itYear.value().end();
                 ++itDate) {
                if (!inventoryDeported[itYear.key()].contains(itDate.key())) {
                    inventoryDeported[itYear.key()][itDate.key()] = itDate.value();
                } else {
                    for (auto itCountryFrom = itDate.value().begin();
                         itCountryFrom != itDate.value().end();
                         ++itCountryFrom) {
                        if (!inventoryDeported[itYear.key()][itDate.key()].contains(itCountryFrom.key())) {
                            inventoryDeported[itYear.key()][itDate.key()][itCountryFrom.key()] = itCountryFrom.value();
                        } else {
                            for (auto itCountryTo = itCountryFrom.value().begin();
                                 itCountryTo != itCountryFrom.value().end();
                                 ++itCountryTo) {
                                if (!inventoryDeported[itYear.key()][itDate.key()][itCountryFrom.key()].contains(itCountryTo.key())) {
                                    inventoryDeported[itYear.key()][itDate.key()][itCountryFrom.key()][itCountryTo.key()] = itCountryTo.value();
                                } else {
                                    for (auto itSku = itCountryTo.value().begin();
                                         itSku != itCountryTo.value().end();
                                         ++itSku) {
                                        inventoryDeported[itYear.key()][itDate.key()][itCountryFrom.key()][itCountryTo.key()]
                                                .insert(itSku.key(), itSku.value());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
//----------------------------------------------------------
void OrdersMapping::uniteUncompleteRefund(
        const OrdersMapping &ordersMapping)
{
    for (auto itYear = ordersMapping.refundByDateUncomplete.begin();
         itYear != ordersMapping.refundByDateUncomplete.end();
         ++itYear) {
        if (!refundByDateUncomplete.contains(itYear.key()))  {
            refundByDateUncomplete[itYear.key()] = itYear.value();
        } else {
            for (auto itDate = itYear.value().begin();
                 itDate != itYear.value().end();
                 ++itDate) {
                if (!refundByDateUncomplete[itYear.key()].contains(itDate.key())) {
                    refundByDateUncomplete[itYear.key()][itDate.key()]
                            = itDate.value();
                } else {
                    for (auto it = itDate.value().begin();
                         it != itDate.value().end();
                         ++it) {
                        refundByDateUncomplete[itYear.key()][itDate.key()]
                                .insert(it.key(), it.value());
                    }
                }
            }
        }
    }
}
//----------------------------------------------------------
void OrdersMapping::uniteUncompleteOrders(const OrdersMapping &ordersMapping)
{
    for (auto itYear = ordersMapping.orderByDateUncomplete.begin();
         itYear != ordersMapping.orderByDateUncomplete.end();
         ++itYear) {
        if (!orderByDateUncomplete.contains(itYear.key()))  {
            orderByDateUncomplete[itYear.key()] = itYear.value();
        } else {
            for (auto itDate = itYear.value().begin();
                 itDate != itYear.value().end();
                 ++itDate) {
                if (!orderByDateUncomplete[itYear.key()].contains(itDate.key())) {
                    orderByDateUncomplete[itYear.key()][itDate.key()] = itDate.value();
                } else {
                    for (auto it = itDate.value().begin();
                         it != itDate.value().end();
                         ++it) {
                        orderByDateUncomplete[itYear.key()][itDate.key()]
                                .insert(it.key(), it.value());
                    }
                }
            }
        }
    }
}
//----------------------------------------------------------
void OrdersMapping::computeOrderNumbers()
{
    ordersQuantityByDate.clear();
    for (auto it = orderById.begin(); it != orderById.end(); ++it) {
        auto order = it.value();
        QDateTime dateTime = order->getDateTime();
        if (dateTime.isNull()) {
            if (order->getShipmentCount() == 0) {
                continue; // TODO investigate + log
            }
            dateTime = order->getShipmentFirst()->getDateTime();
            Q_ASSERT(dateTime.isValid());
        }
        int year = dateTime.date().year();
        if (!ordersQuantityByDate.contains(year)) {
            ordersQuantityByDate[year] = QHash<QString, QMap<QDate, int>>();
        }
        if (!ordersQuantityByDate[year].contains(order->getSubchannel())) {
            ordersQuantityByDate[year][order->getSubchannel()] = QMap<QDate, int>();
        }
        if (!ordersQuantityByDate[year][order->getSubchannel()].contains(dateTime.date())) {
            ordersQuantityByDate[year][order->getSubchannel()][dateTime.date()] = 1;
        } else {
            ordersQuantityByDate[year][order->getSubchannel()][dateTime.date()]++;
        }
    }
    for (auto itYear = orderByDateUncomplete.begin(); itYear != orderByDateUncomplete.end(); ++itYear) {
        int year = itYear.key();
        if (!ordersQuantityByDate.contains(year)) {
            ordersQuantityByDate[year] = QHash<QString, QMap<QDate, int>>();
        }
        for (auto itSubChannel = itYear.value().begin(); itSubChannel != itYear.value().end(); ++itSubChannel) {
            QString subChannel = itSubChannel.key();
            if (!ordersQuantityByDate[year].contains(subChannel)) {
                ordersQuantityByDate[year][subChannel] = QMap<QDate, int>();
            }
            for (auto itDate = itSubChannel.value().begin(); itDate != itSubChannel.value().end(); ++itDate) {
                auto dateTime = itDate.key();
                auto date = dateTime.date();
                for (auto itOrder = itDate.value().begin(); itOrder != itDate.value().end(); ++itOrder) {
                    if (!ordersQuantityByDate[year][subChannel].contains(dateTime.date())) {
                        ordersQuantityByDate[year][subChannel][date] = 1;
                    } else {
                        ordersQuantityByDate[year][subChannel][date]++;
                    }
                }
            }
        }
    }
}
//----------------------------------------------------------
void OrdersMapping::createRefundsFromUncomplete()
{
    for (auto itYear = refundByDateUncomplete.begin();
         itYear != refundByDateUncomplete.end(); ++itYear) {
        for (auto itReportType = itYear.value().begin();
             itReportType != itYear.value().end(); ++itReportType) {
            for (auto itDate = itReportType.value().begin();
                 itDate != itReportType.value().end(); ++itDate) {
                auto dateTime = itDate.key();
                for (auto itOrder = itDate.value().begin();
                     itOrder != itDate.value().end(); ++itOrder) {
                    QString orderId = itOrder.key();
                    QHash<QString, int> idRefundByOrder;
                    if (orderById.contains(orderId)) {
                        if (!idRefundByOrder.contains(orderId)) {
                            idRefundByOrder[orderId] = 1;
                        } else {
                            ++idRefundByOrder[orderId];
                        }
                        QString refundId = orderId + "-ref-" + dateTime.toString("yyyy-MM-dd")
                                + "-" + QString::number(idRefundByOrder[orderId]);
                        auto order = orderById[orderId];
                        double orderTotal = order->getTotalPriceTaxed();
                        double refundTotal = itOrder.value();
                        Shipping finalShipping = order->getShipping();
                        auto shipments = order->getShipments();
                        for (auto it = shipments.begin(); it != shipments.end(); ++it) {
                            finalShipping.addCost(it.value()->getShipping());
                        }
                        QSharedPointer<Refund> refund(nullptr);
                        if (qAbs(orderTotal+refundTotal) < 0.01) {
                            QHash<QString, QSharedPointer<ArticleSold> > articlesSoldCopy;
                            auto articlesSold = order->articlesSoldUnited();
                            for (auto it = articlesSold.begin(); it != articlesSold.end(); ++it) {
                                QSharedPointer<ArticleSold> articleCopy(
                                            new ArticleSold(
                                                it.key(),
                                                it.value()->getSku(),
                                                it.value()->getName(),
                                                it.value()->getSaleType(),
                                                it.value()->getUnits(),
                                                it.value()->getTotalPriceTaxed(),
                                                it.value()->getTotalPriceTaxes(),
                                                it.value()->getCurrency()
                                                ));
                                articlesSoldCopy[it.key()] = articleCopy;
                            }
                            refund = QSharedPointer<Refund>(
                                        new Refund(
                                            refundId,
                                            orderId,
                                            articlesSoldCopy,
                                            finalShipping,
                                            dateTime,
                                            Address(),
                                            order->getCurrency(),
                                            order->getVatForRoundCorrection()));
                        } else {
                            QList<QSharedPointer<ArticleSold>> articlesByVatSorted;
                            QList<double> vatRates;
                            QHash<QString, QSharedPointer<ArticleSold>> articlesSoldOrder
                                    = order->articlesSold();
                            QHash<QString, QSharedPointer<ArticleSold>> articlesPartialRefund;
                            for (auto it = articlesSoldOrder.begin(); it != articlesSoldOrder.end(); ++it) {
                                QString sku = it.value()->getSku();
                                double vatRate //TODO here the problem is that we don't know yet the vat country
                                        = VatRateManager::instance()->vatRate(
                                            ShippingAddressesManager::instance()->companyCountryCode(),
                                            dateTime.date(),
                                            sku);
                                int position = 0;
                                while (position < vatRates.size()
                                       && vatRates[position] > vatRate) {
                                    ++position;
                                }
                                vatRates.insert(position, vatRate);
                                articlesByVatSorted.insert(position, it.value());
                            }
                            double currentRefundAmount = 0.;
                            if (qAbs(finalShipping.totalPriceTaxed()) > qAbs(refundTotal)) {
                                double newTaxes = finalShipping.totalPriceTaxed() * refundTotal / finalShipping.totalPriceTaxed();
                                finalShipping.setTotalPriceTaxed(refundTotal);
                                finalShipping.setTotalTaxes(newTaxes);
                                currentRefundAmount = refundTotal;
                            } else {
                                currentRefundAmount = finalShipping.totalPriceTaxed();
                            }
                            for (auto it = articlesSoldOrder.begin(); it != articlesSoldOrder.end(); ++it) {
                                if (qAbs(currentRefundAmount) >= qAbs(refundTotal)) {
                                    break;
                                }
                                if (qAbs(currentRefundAmount) + qAbs(it.value()->getTotalPriceTaxed()) > qAbs(refundTotal)) {
                                    double articleAmount = refundTotal - currentRefundAmount;
                                    double articleTaxes = it.value()->getTotalPriceTaxes() * articleAmount / it.value()->getTotalPriceTaxed();
                                    QSharedPointer<ArticleSold> articleCopy(
                                                new ArticleSold(
                                                    it.key(),
                                                    it.value()->getSku(),
                                                    it.value()->getName(),
                                                    it.value()->getSaleType(),
                                                    it.value()->getUnits(),
                                                    articleAmount,
                                                    articleTaxes,
                                                    it.value()->getCurrency()
                                                    ));

                                    articleCopy->init(it.value()->getShipment());
                                    // TODO init from shipment ?
                                    articlesPartialRefund[it.key()] = articleCopy;
                                    currentRefundAmount += articleCopy->getTotalPriceTaxed();
                                    break;
                                } else {
                                    QSharedPointer<ArticleSold> articleCopy(
                                                new ArticleSold(
                                                    it.key(),
                                                    it.value()->getSku(),
                                                    it.value()->getName(),
                                                    it.value()->getSaleType(),
                                                    it.value()->getUnits(),
                                                    it.value()->getTotalPriceTaxed(),
                                                    it.value()->getTotalPriceTaxes(),
                                                    it.value()->getCurrency()
                                                    ));

                                    articlesPartialRefund[it.key()] = articleCopy;
                                    currentRefundAmount += articleCopy->getTotalPriceTaxed();
                                }
                            }
                            refund = QSharedPointer<Refund>(
                                        new Refund(
                                            refundId,
                                            orderId,
                                            articlesPartialRefund,
                                            finalShipping,
                                            dateTime,
                                            Address(),
                                            order->getCurrency(),
                                            order->getVatForRoundCorrection()));
                            refund->setWasGuessed(true);
                            double refundTotalCreated = refund->getTotalPriceTaxed();
                            Q_ASSERT(qAbs(refundTotalCreated) - qAbs(refundTotal) < 0.001);
                        }
                        refund->addReportFrom(maxDate.date(), itReportType.key());
                        refund->reversePrices();
                        if (refund->skusSet().isEmpty()) {
                            refund->setRetrieveAllShipmentsOnInit(true);
                        }
                        if (refundId == "21121201486VU65"
                                || orderId == "21121201486VU65") {
                            int TEMP=10;++TEMP;
                        }
                        refund->init(order.data());
                        refund->setChannel(order->getChannel());
                        refund->setSubchannel(order->getSubchannel());
                        int refundYear = dateTime.date().year();
                        if (!refundByDate.contains(refundYear)) {
                            refundByDate[refundYear]
                                    = QMultiMap<QDateTime, QSharedPointer<Refund>>();
                        }
                        refundByDate[refundYear].insert(dateTime, refund);
                        refundByOrderId.insert(orderId, refund);
                        refundById.insert(refundId, refund);
                    }
                }
            }
        }
    }
}
//----------------------------------------------------------
void OrdersMapping::initOrdersUncomplete(
        const QDateTime &dateTime, const QString &subChannel)
{
    int year = dateTime.date().year();
    if (!orderByDateUncomplete.contains(year)) {
        orderByDateUncomplete[year]
                = QHash<QString, QMap<QDateTime, QHash<QString, double>>>();
    }
    if (!orderByDateUncomplete[year].contains(subChannel)) {
        orderByDateUncomplete[year][subChannel]
                = QMap<QDateTime, QHash<QString, double>>();
    }
    if (!orderByDateUncomplete[year][subChannel].contains(dateTime)) {
        orderByDateUncomplete[year][subChannel][dateTime]
                = QHash<QString, double>();
    }
}
//----------------------------------------------------------
void OrdersMapping::initRefundsUncomplete(
        const QDateTime &dateTime, const QString &reportType)
{
    int year = dateTime.date().year();
    if (!refundByDateUncomplete.contains(year)) {
        refundByDateUncomplete[year]
                = QHash<QString, QMap<QDateTime, QHash<QString, double>>>();
    }
    if (!refundByDateUncomplete[year].contains(reportType)) {
        refundByDateUncomplete[year][reportType]
                = QMap<QDateTime, QHash<QString, double>>();
    }
    if (!refundByDateUncomplete[year][reportType].contains(dateTime)) {
        refundByDateUncomplete[year][reportType][dateTime]
                = QHash<QString, double>();
    }
}
//----------------------------------------------------------
//----------------------------------------------------------
