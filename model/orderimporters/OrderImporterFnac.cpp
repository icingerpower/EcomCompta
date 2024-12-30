#include "../common/utils/CsvReader.h"

#include "OrderImporterFnac.h"
#include "model/bookkeeping/ManagerSaleTypes.h"
#include "model/orderimporters/ShippingAddressesManager.h"


const QString OrderImporterFnac::NAME = "Fnac";
const QString OrderImporterFnac::SUB_CHANNEL = "fnac.com";
const QString OrderImporterFnac::REPORT_SALES_MERCHENT
= QObject::tr("Rapport des commandes");
const QString OrderImporterFnac::REPORT_PAYMENTS
= QObject::tr("Rapport des transactions");
//----------------------------------------------------------
OrderImporterFnac::OrderImporterFnac()
    : AbstractOrderImporter()
{
}
//----------------------------------------------------------
OrderImporterFnac::~OrderImporterFnac()
{
}
//----------------------------------------------------------
QString OrderImporterFnac::name() const
{
    return NAME;
}
//----------------------------------------------------------
QString OrderImporterFnac::invoicePrefix() const
{
    return "FNAC";
}
//----------------------------------------------------------
QString OrderImporterFnac::uniqueId() const
{
    return "OrderImporterFnac";
}
//----------------------------------------------------------
QList<ReportType> OrderImporterFnac::reportTypes() const
{
    QList<ReportType> reportTypes;
    ReportType typeSalesMerchent;
    typeSalesMerchent.shortName = REPORT_SALES_MERCHENT;
    typeSalesMerchent.extensions = QStringList({"csv"});
    reportTypes << typeSalesMerchent;
    ReportType typePayment;
    typePayment.shortName = REPORT_PAYMENTS;
    typePayment.extensions = QStringList({"csv"});
    reportTypes << typePayment;
    return reportTypes;
}
//----------------------------------------------------------
QSharedPointer<OrdersMapping> OrderImporterFnac::loadReport(
        const QString &reportTypeName,
        const QString &fileName,
        int maxYear) const
{
    QSharedPointer<OrdersMapping> orderMapping;
    if (reportTypeName == REPORT_SALES_MERCHENT) {
        orderMapping = _loadReportSalesMerchent(fileName, maxYear);
    } else if (reportTypeName == REPORT_PAYMENTS) {
        orderMapping = _loadReportPayments(fileName, maxYear);
    }
    for (auto order : orderMapping->orderById) {
        for (auto shipment : order->getShipments()) {
            shipment->addReportFrom(orderMapping->maxDate.date(), reportTypeName);
        }
    }
    return orderMapping;
}
//----------------------------------------------------------
QList<QStringList> OrderImporterFnac::reportForOrderComplete(const Order *) const
{
    static QList<QStringList> reports
            = {
        {REPORT_SALES_MERCHENT}
        , {REPORT_SALES_MERCHENT, REPORT_PAYMENTS}
    };
    return reports;
}
//----------------------------------------------------------
QSharedPointer<OrdersMapping> OrderImporterFnac::_loadReportSalesMerchent(
        const QString &fileName, int maxYear) const
{
    QSharedPointer<OrdersMapping> orderMapping(
                new OrdersMapping);
    CsvReader reader(fileName, ";", "\"", true, "\r\n");
    QString relFileName = QFileInfo(fileName).fileName();
    reader.readAll();
    const DataFromCsv *dataRode = reader.dataRode();
    int indOrderId = dataRode->header.pos("Commande");
    QString subChannel = SUB_CHANNEL;
    int indDateTime = dataRode->header.pos("Date de création");
    int indOrderStatus = dataRode->header.pos("Statut commande");
    int indProductName = dataRode->header.pos("Produits");
    //int indEan = dataRode->header.pos("EAN");
    int indSKU = dataRode->header.pos("SKUs");
    int indQty = dataRode->header.pos("Quantité");
    int indPriceProducts = dataRode->header.pos("Prix produits (TTC)");
    int indPriceShipping = dataRode->header.pos("Prix frais de port (TTC)");
    //int indPriceTotalPaid = dataRode->header.pos("Prix total (TTC)");
    int indPriceTotalPaidVat = dataRode->header.pos("TVA produits et frais de port");
    int indFirstName = dataRode->header.pos("Prénom livraison");
    int indLastName = dataRode->header.pos("Nom livraison");
    int indStreet1 = dataRode->header.pos("Adresse 1 de livraison");
    int indStreet2 = dataRode->header.pos("Adresse 2 de livraison");
    int indStreet3 = dataRode->header.pos("Adresse 3 de livraison");
    int indPostalCode = dataRode->header.pos("Code postal de livraison");
    int indCity = dataRode->header.pos("Ville de livraison");
    int indCountryCode = dataRode->header.pos("Pays livraison");
    int indFirstNameInvoice = dataRode->header.pos("Prénom facturation");
    int indLastNameInvoice = dataRode->header.pos("Nom facturation");
    int indStreet1Invoice = dataRode->header.pos("Adresse 1 facturation");
    int indStreet2Invoice = dataRode->header.pos("Adresse 2 facturation");
    int indStreet3Invoice = dataRode->header.pos("Adresse 3 facturation");
    int indPostalCodeInvoice = dataRode->header.pos("Code postal de facturation");
    int indCityInvoice = dataRode->header.pos("Ville facturation");
    int indCountryCodeInvoice = dataRode->header.pos("Pays facturation");
    for (auto elements : dataRode->lines) {
        if (elements[indOrderStatus] == "Reçue") {
            QDateTime dateTimeOrder
                    = QDateTime::fromString(
                        elements[indDateTime],
                        "dd/MM/yyyy");
            Q_ASSERT(dateTimeOrder.isValid());
            if (dateTimeOrder.date().year() > maxYear) {
                continue;
            }
            orderMapping->minDate = qMin(dateTimeOrder, orderMapping->minDate);
            orderMapping->maxDate = qMax(dateTimeOrder, orderMapping->maxDate);
            QString orderId = elements[indOrderId];
            QString currency = "EUR";
            QString buyerName
                    = elements[indFirstName] + " " + elements[indLastName];
            QString countryCode3letters = elements[indCountryCode];
            QString countryCode = countryCode3letters.left(2);
            Address addressTo(buyerName,
                              elements[indStreet1],
                              elements[indStreet2],
                              elements[indStreet3],
                              elements[indCity],
                              elements[indPostalCode],
                              countryCode,
                              "",
                              "");
            QString buyerNameInvoice
                    = elements[indFirstNameInvoice] + " " + elements[indLastNameInvoice];
            QString countryCodeInvoice = elements[indCountryCodeInvoice].left(2);
            Address addressToInvoice(buyerNameInvoice,
                                     elements[indStreet1Invoice],
                                     elements[indStreet2Invoice],
                                     elements[indStreet3Invoice],
                                     elements[indCityInvoice],
                                     elements[indPostalCodeInvoice],
                                     countryCodeInvoice,
                                     "",
                                     "");
            Shipping shippingExtra(0., 0., currency);
            QString sku = elements[indSKU];
            QString articleShipmentId = orderId + "-" + sku;
            int quantityShipped = elements[indQty].toInt();
            //Q_ASSERT(quantityShipped == 1); // TODO remove as one crash to check order
            QString totalPriceTaxedStr = elements[indPriceProducts].replace(",", ".");
            double totalPriceTaxed = totalPriceTaxedStr.toDouble();
            QString totalShippingTaxedStr = elements[indPriceShipping].replace(",", ".");
            double totalShippingTaxed = totalShippingTaxedStr.toDouble();
            QString vatTotalStr = elements[indPriceTotalPaidVat].replace(",", ".");
            double vatTotal = vatTotalStr.toDouble();
            Shipping shippingArticle(totalShippingTaxed, 0., currency);
            QHash<QString, QSharedPointer<ArticleSold>> articles;
            QSharedPointer<ArticleSold> article(
                        new ArticleSold(
                            articleShipmentId,
                            sku,
                            elements[indProductName],
                            ManagerSaleTypes::SALE_PRODUCTS,
                            quantityShipped,
                            totalPriceTaxed,
                            0.,
                            currency));
            article->setShipping(shippingArticle);
            articles[article->getShipmentItemId()] = article;
            QString addressId = getDefaultShippingAddressId();;
            Address addressFrom = ShippingAddressesManager::instance()->getAddress(addressId);
            QDateTime dateTimeShipping = dateTimeOrder; /// To make things easier, we do as order were shipped same day
            QString shipmentId = orderId;  /// As shipment ID we put order ID assuming all orders will be shipped in once which could be wrong sometimes
            QSharedPointer<Shipment> shipment(
                        new Shipment(shipmentId,
                                     articles,
                                     Shipping(0., 0., currency),
                                     dateTimeShipping,
                                     addressFrom,
                                     currency,
                                     vatTotal));
            QHash<QString, QSharedPointer<Shipment>> shipments;
            shipments[shipment->getId()] = shipment;
            QSharedPointer<Order> order(
                        new Order(
                            dateTimeOrder,
                            orderId,
                            currency,
                            name(),
                            subChannel,
                            addressTo,
                            addressToInvoice,
                            shippingExtra,
                            true,
                            shipments));
            bool isBusinessCustomer = false;
            order->setIsBusinessCustomer(isBusinessCustomer);
            if (isVatToRecompute()) {
                order->setVatToRecompute(true);
            }
            int yearShipment = dateTimeShipping.date().year(); /// Amazon use shippig date for invoicne / vat report
            if (orderMapping->orderById.contains(orderId)) {
                orderMapping->orderById[orderId]->merge(*order.data());
            } else {
                orderMapping->orderById[orderId] = order;
                int yearOrder = dateTimeShipping.date().year();
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
                        .insert(dateTimeShipping, shipment);
            }
        }
    }
    orderMapping->removeEmptyYears();
    if (orderMapping->minDate > orderMapping->maxDate) {
        orderMapping->minDate = QDateTime();
        orderMapping->maxDate = QDateTime();
    }
    return orderMapping;
}
//----------------------------------------------------------
QSharedPointer<OrdersMapping> OrderImporterFnac::_loadReportPayments(
        const QString &fileName, int maxYear) const
{
    QSharedPointer<OrdersMapping> orderMapping(new OrdersMapping);
    CsvReader reader(fileName, ";", "\"", true, "\r\n", 0, "ISO 8859-1");
    reader.readAll();
    const DataFromCsv *dataRode = reader.dataRode();
    int indName = dataRode->header.pos("Produit");
    int indOrderId = dataRode->header.pos("N° commande logistique");
    int indDateTimeAccepted = dataRode->header.pos("Acceptée le");
    int indDateTimeCreated = dataRode->header.pos("Créée le");
    int indAmount = dataRode->header.pos("Montant");

    QHash<QString, /// order-id
            QHash<QString, /// refund-id
            QHash<QDateTime, /// refunddate
            QHash<QString, double>>>> refundInfos; /// amount-name
    QHash<QString, /// order-id
            QHash<QDateTime, /// refunddate
            QHash<QString, double>>> orderInfos; /// amount-name
    for (auto elements : dataRode->lines) {
        QString name = elements[indName];
        QString orderId = elements[indOrderId].trimmed();
        QDateTime dateTime;
        QDateTime dateTimeAccepted;
        QDateTime dateTimeCreated;
        if (!elements[indDateTimeAccepted].trimmed().isEmpty()) {
            dateTimeAccepted = QDateTime::fromString(elements[indDateTimeAccepted], "dd/MM/yyyy");
            dateTime = dateTimeAccepted;
            Q_ASSERT(dateTime.isValid());
        } else if (!elements[indDateTimeCreated].trimmed().isEmpty()) {
            dateTimeCreated = QDateTime::fromString(elements[indDateTimeCreated], "dd/MM/yyyy");
            dateTime = dateTimeCreated;
        }
        if (dateTime.date().year() > maxYear) {
            continue;
        }
        if (dateTime.isValid()) {
            orderMapping->minDate = qMin(dateTime, orderMapping->minDate);
            orderMapping->maxDate = qMax(dateTime, orderMapping->maxDate);
        }
        if (name.startsWith("Remboursement")
                && !orderId.isEmpty()) {
                //&& !name.contains("omission")) {
            QString refundId = orderId + "_" + dateTime.toString("yyyy-MM-dd");
            QString amountStr = elements[indAmount].replace(",", ".");;
            double amount = amountStr.toDouble();
            if (!refundInfos.contains(orderId)) {
                refundInfos[orderId]
                        = QHash<QString, QHash<QDateTime, QHash<QString, double>>>();
            }
            if (!refundInfos[orderId].contains(refundId)) {
                refundInfos[orderId][refundId]
                        = QHash<QDateTime, QHash<QString, double>>();
            }
            if (!refundInfos[orderId][refundId].contains(dateTime)) { //TODO issue if 2 refund in same date
                refundInfos[orderId][refundId][dateTime]
                        = QHash<QString, double>();
            }
            refundInfos[orderId][refundId][dateTime][name] = amount;
        } else if (!orderId.isEmpty()) {
            QString amountStr = elements[indAmount].replace(",", ".");;
            double amount = amountStr.toDouble();
            if (!orderInfos.contains(orderId)) {
                orderInfos[orderId]
                        = QHash<QDateTime, QHash<QString, double>>();
            }
            if (!orderInfos[orderId].contains(dateTime)) { //TODO issue if 2 order in same date
                orderInfos[orderId][dateTime]
                        = QHash<QString, double>();
            }
            orderInfos[orderId][dateTime][name] = amount;
        }
    }
    for (auto itRefund = refundInfos.begin();
         itRefund != refundInfos.end(); ++itRefund) {
        for (auto itRefundId = itRefund.value().begin();
             itRefundId != itRefund.value().end(); ++itRefundId) {
            for (auto itDate = itRefundId.value().begin();
                 itDate != itRefundId.value().end(); ++itDate) {
                QHash<QString, QSharedPointer<ArticleSold>> articlesSold;
                double totalRefund = 0.;
                //double amountShippingTaxed = 0.;
                //QString productName;
                for (auto itAmount = itDate.value().begin();
                     itAmount != itDate.value().end(); ++itAmount) {
                    if (!itAmount.key().contains("omission")) {
                        totalRefund += itAmount.value();
                    }
                    /*
                    if (itAmount.key().contains("Frais de port")) {
                        amountShippingTaxed = itAmount.value();
                    }
                    if (itAmount.key().startsWith("Remboursement\\")) {
                        productName = itAmount.key();
                    }
                    //*/
                }
                QDateTime dateTime = itDate.key();
                QString orderId = itRefund.key();
                int yearRefund = itDate.key().date().year();
                if(!orderMapping->refundByDateUncomplete.contains(yearRefund)) {
                    orderMapping->refundByDateUncomplete[yearRefund]
                            = QHash<QString, QMap<QDateTime, QHash<QString, double>>>();
                    orderMapping->refundByDateUncomplete[yearRefund][REPORT_PAYMENTS]
                            = QMap<QDateTime, QHash<QString, double>>();
                }
                if(!orderMapping->refundByDateUncomplete[yearRefund][REPORT_PAYMENTS].contains(dateTime)) {
                    orderMapping->refundByDateUncomplete[yearRefund][REPORT_PAYMENTS][dateTime]
                            = QHash<QString, double>();
                }
                if (orderMapping->refundByDateUncomplete[yearRefund][REPORT_PAYMENTS][dateTime].contains(orderId)) {
                    orderMapping->refundByDateUncomplete[yearRefund][REPORT_PAYMENTS][dateTime][orderId]
                            += totalRefund;
                } else {
                    orderMapping->refundByDateUncomplete[yearRefund][REPORT_PAYMENTS][dateTime].insert(
                                orderId, totalRefund);
                }
            }
        }
    }
    for (auto itOrder = orderInfos.begin();
         itOrder != orderInfos.end(); ++itOrder) {
        for (auto itDate = itOrder.value().begin();
             itDate != itOrder.value().end(); ++itDate) {
            QHash<QString, QSharedPointer<ArticleSold>> articlesSold;
            double totalAmount = 0.;
            //double amountShippingTaxed = 0.;
            //QString productName;
            for (auto itAmount = itDate.value().begin();
                 itAmount != itDate.value().end(); ++itAmount) {
                if (!itAmount.key().contains("omission")) {
                    totalAmount += itAmount.value();
                }
            }
            QDateTime dateTime = itDate.key();
            QString orderId = itOrder.key();
            int year = dateTime.date().year();
            orderMapping->initOrdersUncomplete(
                        dateTime, SUB_CHANNEL);
            orderMapping->orderByDateUncomplete
                    [year][SUB_CHANNEL][dateTime][orderId] = totalAmount;
        }
    }

    if (!orderMapping->minDate.isValid() || !orderMapping->minDate.isValid()) {
        int TEMP=10;++TEMP;
    }
    if (orderMapping->minDate > orderMapping->maxDate) {
        orderMapping->minDate = QDateTime();
        orderMapping->maxDate = QDateTime();
    }
    return orderMapping;
}
//----------------------------------------------------------
