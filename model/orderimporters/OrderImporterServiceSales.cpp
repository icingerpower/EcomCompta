#include "ServiceSalesModel.h"
#include "AddressesServiceCustomer.h"
#include "model/bookkeeping/ManagerSaleTypes.h"

#include "OrderImporterServiceSales.h"

const QString OrderImporterServiceSales::NAME = "Ads self invoicing";
const QString OrderImporterServiceSales::FILE_SALES = "csv sale file";
//----------------------------------------------------------
OrderImporterServiceSales::OrderImporterServiceSales()
    : AbstractOrderImporter()
{

}
//----------------------------------------------------------
QString OrderImporterServiceSales::uniqueId() const
{
    return "OrderImporterSelfInvoicingSales";
}
//----------------------------------------------------------
QString OrderImporterServiceSales::name() const
{
    return NAME;
}
//----------------------------------------------------------
QString OrderImporterServiceSales::invoicePrefix() const
{
    return "ADS";
}
//----------------------------------------------------------
QList<ReportType> OrderImporterServiceSales::reportTypes() const
{
    QList<ReportType> reportTypes;
    ReportType typeSalesAppFile;
    typeSalesAppFile.shortName = FILE_SALES;
    typeSalesAppFile.extensions = QStringList({"csv"});
    reportTypes << typeSalesAppFile;
    return reportTypes;
}
//----------------------------------------------------------
QList<QStringList> OrderImporterServiceSales::reportForOrderComplete(
        const Order *order) const
{
    static QList<QStringList> reports
            = {
        {FILE_SALES}
    };
    return reports;
}
//----------------------------------------------------------
QSharedPointer<OrdersMapping> OrderImporterServiceSales::loadReport(
        const QString &reportTypeName,
        const QString &filePath,
        int maxYear) const
{
    QSharedPointer<OrdersMapping> orderMapping(new OrdersMapping);
    auto lineSales = ServiceSalesModel::instance()
            ->loadListOfVariantList(filePath);
    for (auto itLine = lineSales.begin();
         itLine != lineSales.end(); ++itLine) {
        QDateTime dateTimeOrder(
                    itLine->value(
                        ServiceSalesModel::IND_COL_DATE).toDate(),
                    QTime(0, 0));
        if (dateTimeOrder.date().year() > maxYear) {
            continue;
        }
        Q_ASSERT(dateTimeOrder.isValid());
        orderMapping->minDate = qMin(dateTimeOrder, orderMapping->minDate);
        orderMapping->maxDate = qMax(dateTimeOrder, orderMapping->maxDate);
        QString orderId = itLine->value(
                    ServiceSalesModel::IND_COL_REFERENCE).toString();
        double amountUnit = itLine->value(
                    ServiceSalesModel::IND_COL_AMOUNT).toDouble();
        int units = itLine->value(
                    ServiceSalesModel::IND_COL_UNIT).toInt();
        QString serviceTitle = itLine->value(
                    ServiceSalesModel::IND_COL_TITLE).toString();
        if (amountUnit < 0.) {
            Q_ASSERT(false); // TODO handle refund
        }
        QString currency = itLine->value(
                    ServiceSalesModel::IND_COL_CURRENCY).toString();
        QString customerId = itLine->value(
                    ServiceSalesModel::IND_COL_ADDRESS_ID).toString();
        Address addressTo = AddressesServiceCustomer::instance()
                ->getAddress(customerId);
        Q_ASSERT(!addressTo.countryCode().isEmpty());
        //ServiceAccounts serviceAccounts(addressTo.internalId());
        // TODO In invoice generation, I have to add a note that a service is to pay on money collected
        // then adapt invoice generation and accounts on compute vat
        QHash<QString, QSharedPointer<ArticleSold>> articles;
        QSharedPointer<ArticleSold> article(
                    new ArticleSold(
                        "affads",
                        "affads",
                        serviceTitle,
                        ManagerSaleTypes::SALE_SERVICES,
                        units,
                        amountUnit * units,
                        0.,
                        currency));
        article->setShipping(Shipping(0., 0., currency));
        articles[article->getShipmentItemId()] = article;
        QString addressId = getDefaultShippingAddressId();;
        Address addressFrom = ShippingAddressesManager::instance()->getAddress(addressId);
        QString shipmentId = orderId;
        QSharedPointer<Shipment> shipment(
                    new Shipment(orderId,
                                 articles,
                                 Shipping(0., 0., currency),
                                 dateTimeOrder,
                                 addressFrom,
                                 currency,
                                 0.));
        QHash<QString, QSharedPointer<Shipment>> shipments;
        shipments[shipment->getId()] = shipment;
        QSharedPointer<Order> order(
                    new Order(
                        dateTimeOrder,
                        orderId,
                        currency,
                        name(),
                        QString(),
                        addressTo,
                        addressTo,
                        Shipping(0., 0., currency),
                        true,
                        shipments));
        order->setIsBusinessCustomer(true);
        order->setVatNumber(addressTo.guessVatNumberFromStreet());
        if (isVatToRecompute()) {
            order->setVatToRecompute(true);
        }
        int yearShipment = dateTimeOrder.date().year(); /// Amazon use shippig date for invoicne / vat report
        if (orderMapping->orderById.contains(orderId)) {
            orderMapping->orderById[orderId]->merge(*order.data());
        } else {
            orderMapping->orderById[orderId] = order;
            int yearOrder = dateTimeOrder.date().year();
            orderMapping->orderByDate[yearOrder].insert(dateTimeOrder, order);
            if (!orderMapping->ordersQuantityByDate[yearOrder].contains(order->getSubchannel())) {
                orderMapping->ordersQuantityByDate[yearOrder][order->getSubchannel()] = QMap<QDate, int>();
            }
            if (!orderMapping->ordersQuantityByDate[yearOrder][order->getSubchannel()].contains(dateTimeOrder.date())) {
                orderMapping->ordersQuantityByDate[yearOrder][order->getSubchannel()][dateTimeOrder.date()] = 1;
            } else {
                orderMapping->ordersQuantityByDate[yearOrder][order->getSubchannel()][dateTimeOrder.date()] += 1;
            }
        }
        if (orderMapping->shipmentById.contains(shipmentId)) {
            orderMapping->shipmentById[shipmentId]->merge(
                        *shipment.data()); // should not be needed as done by order->merge
        } else {
            orderMapping->shipmentById[shipmentId] = shipment;
            orderMapping->shipmentByDate[yearShipment]
                    .insert(dateTimeOrder, shipment);
        }

    }
    orderMapping->removeEmptyYears();
    if (orderMapping->minDate > orderMapping->maxDate) {
        orderMapping->minDate = QDateTime();
        orderMapping->maxDate = QDateTime();
    }
    for (auto order : orderMapping->orderById) {
        for (auto shipment : order->getShipments()) {
            shipment->addReportFrom(
                        orderMapping->maxDate.date(), reportTypeName);
        }
    }
    return orderMapping;
}
//----------------------------------------------------------
