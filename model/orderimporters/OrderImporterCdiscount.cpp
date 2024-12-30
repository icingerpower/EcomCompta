#include "../common/utils/CsvReader.h"

#include "OrderImporterCdiscount.h"
#include "model/orderimporters/ShippingAddressesManager.h"
#include "model/bookkeeping/ManagerSaleTypes.h"

const QString OrderImporterCdiscount::SUB_CHANNEL = "cdiscount.fr";
const QString OrderImporterCdiscount::NAME = "Cdiscount";
const QString OrderImporterCdiscount::REPORT_SALES_MERCHENT
= QObject::tr("Rapport des commandes");
const QString OrderImporterCdiscount::REPORT_PAYMENTS
= QObject::tr("Rapport finance");
//----------------------------------------------------------
//----------------------------------------------------------
OrderImporterCdiscount::OrderImporterCdiscount()
{
}
//----------------------------------------------------------
OrderImporterCdiscount::~OrderImporterCdiscount()
{
}
//----------------------------------------------------------
QString OrderImporterCdiscount::name() const
{
    return NAME;
}
//----------------------------------------------------------
QString OrderImporterCdiscount::invoicePrefix() const
{
    return "CDISCOUNT";
}
//----------------------------------------------------------
QString OrderImporterCdiscount::uniqueId() const
{
    return "OrderImporterCdiscount";
}
//----------------------------------------------------------
QList<ReportType> OrderImporterCdiscount::reportTypes() const
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
QSharedPointer<OrdersMapping> OrderImporterCdiscount::loadReport(
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
            shipment->addReportFrom(
                        orderMapping->maxDate.date(), reportTypeName);
        }
    }
    return orderMapping;
}
//----------------------------------------------------------
QList<QStringList> OrderImporterCdiscount::reportForOrderComplete(const Order *) const
{
    static QList<QStringList> reports
            = {
        {REPORT_SALES_MERCHENT}
        , {REPORT_SALES_MERCHENT, REPORT_PAYMENTS}
    };
    return reports;
}
//----------------------------------------------------------
QSharedPointer<OrdersMapping>
OrderImporterCdiscount::_loadReportSalesMerchent(
        const QString &fileName,
        int maxYear) const
{
    QSharedPointer<OrdersMapping> orderMapping(
                new OrdersMapping);
    CsvReader reader(fileName, ";", "\"", true, "\r\n", 5);
    reader.readAll();
    const DataFromCsv *dataRode = reader.dataRode();
    int indOrderId = dataRode->header.pos({"Référence commande", "Référence commande /\nOrder ID", "Order Number"});
    int indSubchannel = dataRode->header.pos({"Enseigne", "Sales Channel"});
    int indDateTime = dataRode->header.pos({"Date de commande", "Date de commande /\nOrder date", "Order date"});
    int indOrderStatus = dataRode->header.pos({"Statut commande", "Statut commande /\nOrder status", "Order status"});
    int indProductName = dataRode->header.pos({"Détail produit", "Détail produit /\nProduct details", "Product detail"});
    //int indEan = dataRode->header.pos("EAN");
    //int indSKU = dataRode->header.pos("SKU Cdiscount");
    int indSKU = dataRode->header.pos({"Référence vendeur /\nSeller reference", "Seller product reference"});
    int indSKUcdiscount = dataRode->header.pos({"SKU Cdiscount", "SKU"});
    int indQty = dataRode->header.pos({"Quantité", "Quantité /\nQuantity", "Quantity"});
    int indPriceTotal
            = dataRode->header.pos({
                "Prix Total (€ TTC) hors frais de traitement", "Prix Total (€ TTC) hors frais de traitement /\nUnit price (incl. tax) excluded processing fee", "Total price (VAT incl.)"});
    int indShipping
            = dataRode->header.pos({"Montant Frais de livraison (€ TTC)", "Montant Frais de livraison (€ TTC) /\nShipping fee (incl. tax)", "Delivery fees (VAT incl.)"});
    int indFirstName = dataRode->header.pos({"Prénom", "Prénom /\nFirst name", "First Name"});
    int indLastName = dataRode->header.pos({"Nom", "Nom /\nLast name", "Last Name"});
    int indStreet = dataRode->header.pos({"Adresse", "Adresse /\nAddress", "Address"});
    int indPostalCode = dataRode->header.pos({"Code postal de livraison", "Code postal de livraison / \nShipping ZIP code", "Zip code"});
    int indCity = dataRode->header.pos({"Ville de livraison", "Ville de livraison / Shipping city", "Delivery City"});
    int indCountryCode = dataRode->header.pos({"Pays de livraison", "Pays de livraison / Shipping Country", "Delivery Country"});

    int indNameStreetInvoice = dataRode->header.pos({"Nom et Adresse de facturation", "Nom et Adresse de facturation / \nBilling name and address", "Invoice name and address"});
    int indPostalCodeInvoice = dataRode->header.pos({"Code postal de facturation", "Code postal de facturation /\nBilling ZIP code", "Invoice Zip code"});
    int indCityInvoice = dataRode->header.pos({"Ville de facturation", "Ville de facturation /\nBilling city", "Invoice city"});
    int indCountryCodeInvoice = dataRode->header.pos({"Pays de facturation", "Pays de facturation / Billing country", "Invoice country"});
    for (auto elements : dataRode->lines) {
        if (elements[indOrderStatus] != "Annulée") {
        //if (true) {
            QDateTime dateTimeOrder
                    = QDateTime::fromString(
                        elements[indDateTime],
                        "dd/MM/yyyy hh:mm:ss");
            if (!dateTimeOrder.isValid()) {
                dateTimeOrder = QDateTime::fromString(
                            elements[indDateTime],
                            "dd-MM-yyyy hh:mm");
            }
            Q_ASSERT(dateTimeOrder.isValid());
            if (dateTimeOrder.date().year() > maxYear) {
                continue;
            }
            orderMapping->minDate = qMin(dateTimeOrder, orderMapping->minDate);
            orderMapping->maxDate = qMax(dateTimeOrder, orderMapping->maxDate);
            QString orderId = elements[indOrderId];
            QString currency = "EUR";
            QString subchannel = elements[indSubchannel];
            if (subchannel.toLower().contains("cdiscount")) {
                subchannel = SUB_CHANNEL;
            }
            QString buyerName
                    = elements[indFirstName] + " " + elements[indLastName];
            QString countryCode = elements[indCountryCode];
            if (elements[indStreet].contains("Retrait Magasin")
                    || elements[indOrderStatus] == "Annulée"
                    || elements[indOrderStatus] == "Retirée") {
                countryCode = "FR";
            }
            Address addressTo(buyerName,
                              elements[indStreet],
                              "",
                              "",
                              elements[indCity],
                              elements[indPostalCode],
                              countryCode,
                              "",
                              "");
            QStringList elementsInvoiceName
                    = elements[indNameStreetInvoice].split(" ");
            QString buyerNameInvoice;
            QString buyerStreetInvoice;
            for (int i=0; i<3; ++i) {
                if (elementsInvoiceName.size() > 1) {
                    buyerNameInvoice = elementsInvoiceName.takeFirst() + buyerStreetInvoice;
                }
            }
            while (elementsInvoiceName.size() > 0) {
                buyerStreetInvoice = elementsInvoiceName.takeFirst() + buyerStreetInvoice;
            }
            Address addressBillingTo = addressTo;
            if (!elements[indCountryCodeInvoice].isEmpty()) {
                addressBillingTo = Address(buyerNameInvoice,
                                           buyerStreetInvoice,
                                           "",
                                           "",
                                           elements[indCityInvoice],
                                           elements[indPostalCodeInvoice],
                                           elements[indCountryCodeInvoice],
                                           "",
                                           "");
            }
            Shipping shippingExtra(0., 0., currency);
            QString sku = elements[indSKU];
            QString skuCdiscount = elements[indSKUcdiscount];
            if (sku.isEmpty()) {
                sku = skuCdiscount;
            }
            QString articleShipmentId = orderId + "-" + sku;
            QString quantityShippedStr = elements[indQty].split(".")[0].replace(",", ".");
            int quantityShipped = quantityShippedStr.toInt();
            QString totalPriceTaxedStr = elements[indPriceTotal].split(" ")[0].replace(",", ".");
            double totalPriceTaxed = totalPriceTaxedStr.toDouble();
            QString totalShippingTaxedStr = elements[indShipping].split(" ")[0].replace(",", ".");
            double totalShippingTaxed = totalShippingTaxedStr.toDouble();
            Shipping shippingArticle(totalShippingTaxed, 0., currency);
            QHash<QString, QSharedPointer<ArticleSold>> articles;
            QString saleType = ManagerSaleTypes::SALE_PRODUCTS;
            if (orderId == "23050319462SX9Q") {
                int TEMP=10;++TEMP;
            }
            if (sku == "INTERETBCA") {
                saleType = ManagerSaleTypes::SALE_PAYMENT_FASCILITOR;
            } else if(sku.isEmpty() && orderId == "22083122454BO5X") {
                int TEMP=10;++TEMP;
            }
            QSharedPointer<ArticleSold> article(
                        new ArticleSold(
                            articleShipmentId,
                            sku,
                            elements[indProductName],
                            saleType,
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
                                     totalPriceTaxed)); //TODO should be 0
            QHash<QString, QSharedPointer<Shipment>> shipments;
            shipments[shipment->getId()] = shipment;
            QSharedPointer<Order> order(
                        new Order(
                            dateTimeOrder,
                            orderId,
                            currency,
                            name(),
                            subchannel,
                            addressTo,
                            addressBillingTo,
                            shippingExtra,
                            true,
                            shipments));
            bool isBusinessCustomer = false;
            order->setIsBusinessCustomer(isBusinessCustomer);
            /*
        QString vatNumber = elements[indBuyerTaxNumber];
        order->setVatNumber(vatNumber);
        QString companyName = elements[indBuyerCompanyName];
        order->setCompanyName(companyName);
        //*/
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
QSharedPointer<OrdersMapping>
OrderImporterCdiscount::_loadReportPayments(
        const QString &fileName, int maxYear) const
{
    QSharedPointer<OrdersMapping> orderMapping(new OrdersMapping);
    CsvReader reader(fileName, ";", "\"", true, "\r\n", 3);
    reader.readAll();
    const DataFromCsv *dataRode = reader.dataRode();
    //int indDatePaymentAnticipated
            //= dataRode->header.pos(
                //"Date de mise en paiement anticipé");
    int indOrderId = dataRode->header.pos("N° commande / Service");
    int indDateTimePayment = dataRode->header.pos({"Date\r\nvirement", "Date\nvirement"});
    int indDateMisePaymentAnticiped = dataRode->header.pos("Date de mise en paiement anticipé");
    int indSubchannel = dataRode->header.pos("Canal de vente");
    //int indSubchannel = dataRode->header.pos("Canal de vente");
    int indDateTime = dataRode->header.pos("Date opération comptable");
    //int indInvoicePaid = dataRode->header.pos("N° facture/avoir");
    //int indTotalPaid = dataRode->header.pos("Total reçu");
    int indShippingPrice = dataRode->header.pos("Frais de port TTC");
    int indRefund = dataRode->header.pos({"Remboursement TTC hors frais de port", "Remboursement TTC"});
    int indCommissions = dataRode->header.pos("Commission Facilités de paiement");
    int indSalesTTC = dataRode->header.pos("Vente TTC hors frais de port");
    //int indCreditComission = dataRode->header.pos("Avoir commission");
    QHash<QString, double> sumByOrderId;
    QHash<QString, QSet<QDate>> dateByOrderId;
    for (auto elements : dataRode->lines) {
        if (!elements[indDateMisePaymentAnticiped].isEmpty()) {
            QString orderId = elements[indOrderId];
            auto date = QDate::fromString(elements[indDateTime], "dd/MM/yyyy");
            double amountSale = elements[indSalesTTC]
                    .replace(",", ".").toDouble();
            double amountShipping = elements[indShippingPrice]
                    .replace(",", ".").toDouble();
            double amountRefund = elements[indRefund]
                    .replace(",", ".").toDouble();
            double amount = amountSale + amountShipping + amountRefund;
            if (!sumByOrderId.contains(orderId)) {
                sumByOrderId[orderId] = 0.;
            }
            sumByOrderId[orderId] += amount;
            dateByOrderId[orderId] << date;
        }
    }
    QStringList orderIds = sumByOrderId.keys();
    for (auto itId = orderIds.begin();
         itId != orderIds.end(); ++itId) {
        if (qAbs(sumByOrderId[*itId]) > 0.001) {
            sumByOrderId.remove(*itId);
        } else {
            auto setDatesIt = dateByOrderId.constFind(*itId);
            QList<QDate> dates(setDatesIt->cbegin(), setDatesIt->cend());
            std::sort(dates.begin(), dates.end());
            int nDays = dates.first().daysTo(dates.last());
            if (nDays > 1) {
                sumByOrderId.remove(*itId);
            }
        }
    }

            // TODO sum of order, then if any is nul, remove it if it was added.
    for (auto elements : dataRode->lines) {
        //if (!elements[indDatePaymentAnticipated].isEmpty()) {
            QDate date;
            QDateTime dateTime;
            QDate datePayment;
            QDateTime dateTimePayment;
            if (!elements[indDateTimePayment].isEmpty()) {
                datePayment = QDate::fromString(elements[indDateTimePayment], "dd/MM/yyyy");
                Q_ASSERT(datePayment.isValid());
                dateTimePayment.setDate(datePayment);
                orderMapping->maxDate = qMax(dateTimePayment, orderMapping->maxDate);
            }
            if (!elements[indDateTime].isEmpty()) {
                date = QDate::fromString(elements[indDateTime], "dd/MM/yyyy");
                dateTime.setDate(date);
                Q_ASSERT(date.isValid());
                orderMapping->minDate = qMin(dateTime, orderMapping->minDate);
                orderMapping->maxDate = qMax(dateTime, orderMapping->maxDate);
            }
            if (!elements[indShippingPrice].isEmpty()) {
                QString currency = "EUR";
                double amountShipping = elements[indShippingPrice]
                        .replace(",", ".").toDouble();
                QString orderId = elements[indOrderId];
                if (orderId == "23050319462SX9Q") {
                    int TEMP=10;++TEMP;
                }
                QString articleShipmentId = orderId + "-shipping";
                QHash<QString, QSharedPointer<ArticleSold>> articles;
                /*
                QSharedPointer<ArticleSold> article(
                        new ArticleSold(
                            articleShipmentId,
                            QString(),
                            QString(),
                            ManagerSaleTypes::SALE_PRODUCTS,
                            1,
                            0.,
                            0.,
                            currency));
            article->setShipping(shippingArticle);
            articles[article->getShipmentItemId()] = article;
            //*/
                QString addressId = getDefaultShippingAddressId();;
                Address addressFrom = ShippingAddressesManager::instance()->getAddress(addressId);
                QDateTime dateTimeShipping = dateTime; /// To make things easier, we do as order were shipped same day
                if (dateTimeShipping.date().year() > maxYear) {
                    continue;
                }
                if (sumByOrderId.contains(orderId)
                        && qAbs(sumByOrderId[orderId]) < 0.001) {
                    continue;
                }
                QString shipmentId = orderId;  /// As shipment ID we put order ID assuming all orders will be shipped in once which could be wrong sometimes
                QSharedPointer<Shipment> shipment(
                            new Shipment(shipmentId,
                                         articles,
                                         Shipping(0., 0., currency),
                                         dateTimeShipping,
                                         addressFrom,
                                         currency,
                                         0.)); //TODO should be 0
                QString subchannel = elements[indSubchannel];
                if (subchannel.toLower().contains("France")) {
                    subchannel = SUB_CHANNEL;
                }
                QHash<QString, QSharedPointer<Shipment>> shipments;
                //shipments[shipment->getId()] = shipment;
                Shipping shippingOrder(amountShipping, 0., currency);
                QSharedPointer<Order> order(
                            new Order(
                                dateTime,
                                orderId,
                                currency,
                                name(),
                                subchannel,
                                Address(),
                                Address(),
                                shippingOrder,
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
                    orderMapping->orderByDate[yearOrder].insert(dateTime, order);
                    if (!orderMapping->ordersQuantityByDate[yearOrder].contains(order->getSubchannel())) {
                        orderMapping->ordersQuantityByDate[yearOrder][order->getSubchannel()] = QMap<QDate, int>();
                    }
                    if (!orderMapping->ordersQuantityByDate[yearOrder][order->getSubchannel()].contains(dateTime.date())) {
                        orderMapping->ordersQuantityByDate[yearOrder][order->getSubchannel()][dateTime.date()] = 1;
                    } else {
                        orderMapping->ordersQuantityByDate[yearOrder][order->getSubchannel()][dateTime.date()] += 1;
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
            if (!elements[indRefund].isEmpty()) {
                    //|| !elements[indCreditComission].isEmpty()) { /// It means it is an order
                double amountRefund = elements[indRefund]
                        .replace(",", ".").toDouble();
                double amountCommissions = elements[indCommissions]
                        .replace(",", ".").toDouble();
                //double amountCreditRefund = elements[indCreditComission]
                //.replace(",", ".").toDouble();
                double totalRefund = amountRefund + amountCommissions;
                QString orderId = elements[indOrderId];
                if (sumByOrderId.contains(orderId)
                        && qAbs(sumByOrderId[orderId]) > 0.001) {
                    if (orderId == "23050319462SX9Q") {
                        int TEMP=10;++TEMP;
                    }
                    int yearRefund = date.year();
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
            } else if (!elements[indSalesTTC].isEmpty()) {
                QString orderId = elements[indOrderId];
                if (sumByOrderId.contains(orderId)
                        && qAbs(sumByOrderId[orderId]) > 0.001) {
                    orderMapping->initOrdersUncomplete(
                                dateTime, SUB_CHANNEL);
                    if (orderId == "23050319462SX9Q") {
                        int TEMP=10;++TEMP;
                    }
                    double amount = elements[indSalesTTC]
                            .replace(",", ".").toDouble();
                    int year = dateTime.date().year();
                    orderMapping->orderByDateUncomplete
                            [year][SUB_CHANNEL][dateTime][orderId] = amount;
                }

            }
            //}
    }
    if (orderMapping->minDate > orderMapping->maxDate) {
        orderMapping->minDate = QDateTime();
        orderMapping->maxDate = QDateTime();
    }
    return orderMapping;
}
//----------------------------------------------------------
