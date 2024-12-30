#include "../common/countries/CountryManager.h"

#include "OrderImporterAmazon.h"
#include "model/orderimporters/FeesTableModel.h"
#include "model/orderimporters/OrderImporterAmazonUE.h"
#include "model/orderimporters/ShippingAddressesManager.h"
#include "model/bookkeeping/ManagerSaleTypes.h"

//----------------------------------------------------------
const QString OrderImporterAmazon::NAME = QObject::tr("Amazon Hors Europe");
const QString OrderImporterAmazon::REPORT_ORDERS_FBM
= QObject::tr("Rapports sur les commandes", "Amazon report name to find in user interface");
const QString OrderImporterAmazon::REPORT_ORDERS_FBM_SHORT
= QObject::tr("Commandes FBM");
const QString OrderImporterAmazon::REPORT_ORDERS_PAYMENTS
= QObject::tr("Paiements", "Amazon report name to find in user interface");
const QString OrderImporterAmazon::REPORT_ORDERS_INVOICING
= QObject::tr("Expéditions effectuées par Amazon – Facturation des taxes", "Amazon report name to find in user interface");
const QString OrderImporterAmazon::REPORT_ORDERS_INVOICING_SHORT
= QObject::tr("Commandes FBA (facturation)");
//----------------------------------------------------------
OrderImporterAmazon::OrderImporterAmazon()
    : AbstractOrderImporter()
{
}
//----------------------------------------------------------
OrderImporterAmazon::~OrderImporterAmazon()
{
}
//----------------------------------------------------------
QString OrderImporterAmazon::name() const
{
    return OrderImporterAmazon::NAME;
}
//----------------------------------------------------------
QString OrderImporterAmazon::invoicePrefix() const
{
    return QObject::tr("AMZDOM", "As amazon domestic");
}
//----------------------------------------------------------
QString OrderImporterAmazon::uniqueId() const
{
    return "OrderImporterAmazon";
}
//----------------------------------------------------------
QList<ReportType> OrderImporterAmazon::reportTypes() const
{
    QList<ReportType> reportTypes;
    ReportType typeFbaInvoicing;
    typeFbaInvoicing.shortName  = REPORT_ORDERS_INVOICING_SHORT;
    //typeFbaInvoicing.name = REPORT_ORDERS_INVOICING;
    typeFbaInvoicing.extensions = QStringList({"csv", "txt"});
    typeFbaInvoicing.helpText = QObject::tr("Le rapport se trouve dans: Rapport > Expédié par amazon > Ventes > Expéditions effectuées par Amazon – Facturation des taxes");
    reportTypes << typeFbaInvoicing;
    ReportType typeFbmOrders;
    typeFbmOrders.shortName = REPORT_ORDERS_FBM_SHORT;
    //typeFbmOrders.name = REPORT_ORDERS_FBM;
    typeFbmOrders.extensions = QStringList({"csv", "txt"});
    typeFbmOrders.helpText = QObject::tr("Le rapport se trouve dans: Commandes > Rapports sur les commandes (Nouvelles commandes). Il faut ajouter les colonnes «Identificateurs d'impôt de l'acheteur» et «Nom de la société de l'acheteur» dans Commandes > Rapport sur les commandes > Ajouter ou supprimer des colonnes de rapport sur la commande.");
    reportTypes << typeFbmOrders;
    ReportType typePayment;
    typePayment.shortName = REPORT_ORDERS_PAYMENTS;
    //typePayment.name = REPORT_ORDERS_PAYMENTS;
    typePayment.extensions = QStringList({"csv", "txt"});
    typePayment.helpText = QObject::tr("Le rapport se trouve dans: Rapports > Paiements > Tous les relevés > Télécharger le fichier modèle V2 (Fichiers à télécharger toutes les 2 semaines pour chaque amazon)");
    reportTypes << typePayment;
    return reportTypes;
}
//----------------------------------------------------------
QSharedPointer<OrdersMapping> OrderImporterAmazon::loadReport(
        const QString &reportTypeName, const QString &fileName, int maxYear) const
{
    QString relFileName = QFileInfo(fileName).fileName();
    QSharedPointer<OrdersMapping> orderMapping;
    if (reportTypeName == REPORT_ORDERS_INVOICING
            || reportTypeName == REPORT_ORDERS_INVOICING_SHORT) {
        orderMapping = _loadReportInvoicing(fileName, maxYear);
    } else if (reportTypeName == REPORT_ORDERS_FBM
              || reportTypeName == REPORT_ORDERS_FBM_SHORT) {
        orderMapping = _loadReportOrdersFbm(fileName, maxYear);
    } else if (reportTypeName == REPORT_ORDERS_PAYMENTS) {
        orderMapping = _loadReportPayments(fileName, maxYear);
    }
    for (auto itOrder = orderMapping->orderById.begin();
         itOrder != orderMapping->orderById.end(); ++itOrder) {
        updateOrderIfMicroCountry(itOrder->data());
        for (auto shipment : (*itOrder)->getShipments()) {
            shipment->addReportFrom(orderMapping->maxDate.date(), reportTypeName);
        }
    }
    /*
    for (auto order : orderMapping->orderById) {
        for (auto shipment : order->getShipments()) {
            if (shipment->orderId() == "111-7562514-2177011") {
                int TEMP=10;++TEMP;
            }
            shipment->addReportFrom(reportTypeName);
        }
    }
    for (auto itOrder = orderMapping->orderById.begin();
         itOrder != orderMapping->orderById.end(); ++itOrder) {
        updateOrderIfMicroCountry(itOrder->data());
    }
    //*/
    for (auto refund : orderMapping->refundById) {
        refund->addReportFrom(orderMapping->maxDate.date(), reportTypeName);
    }
    return orderMapping;
}
//----------------------------------------------------------
QList<QStringList> OrderImporterAmazon::reportForOrderComplete(const Order *) const
{
    static QList<QStringList> reports
            = {
        {REPORT_ORDERS_INVOICING_SHORT, REPORT_ORDERS_PAYMENTS}
        , {REPORT_ORDERS_INVOICING_SHORT}
        , {REPORT_ORDERS_FBM_SHORT}
        , {REPORT_ORDERS_FBM_SHORT, REPORT_ORDERS_PAYMENTS}
    };
    return reports;
}
//----------------------------------------------------------
QDateTime OrderImporterAmazon::dateTimeFromString(
        const QString &string)
{
    QDateTime dateTime = OrderImporterAmazonUE::dateTimeFromString(string);
    return dateTime;
}
//----------------------------------------------------------
CsvReader OrderImporterAmazon::createAmazonReader(const QString &fileName)
{
    return OrderImporterAmazonUE::createAmazonReader(fileName);
}
//----------------------------------------------------------
Address OrderImporterAmazon::_addressFromSubchannel(
        const QString &subchannel,
        const QString &fbaCenterCode) const
{
    Address address;
    if (subchannel.endsWith(".com")) {
        address.setCountryCode("US");
    } else if (subchannel.endsWith(".ca")) {
        address.setCountryCode("CA");
    } else if (subchannel.endsWith(".au")) {
        address.setCountryCode("AU");
    } else if (subchannel.endsWith(".mx")) {
        address.setCountryCode("MX");
    } else if (subchannel.endsWith(".jp")) {
        address.setCountryCode("JP");
    } else if (subchannel.endsWith(".in")) {
        address.setCountryCode("IN");
    } else if (subchannel.endsWith(".sg")) {
        address.setCountryCode("SG");
    } else {
        Q_ASSERT(false);
    }
    return address;
}
//----------------------------------------------------------
QSharedPointer<OrdersMapping> OrderImporterAmazon::_loadReportOrdersFbm(
        const QString &fileName, int maxYear) const
{
    QSharedPointer<OrdersMapping> orderMapping(new OrdersMapping);
    orderMapping->addYears(12);
    CsvReader reader = createAmazonReader(fileName);
    reader.readAll();
    const DataFromCsv *dataRode = reader.dataRode();

    int indAmazonOrderId = dataRode->header.pos("order-id");
    int indPurchaseDate = dataRode->header.pos({"purchase-date", "Purchase Date"});
    int indBuyerName = dataRode->header.pos("buyer-name");
    int indBuyerPhone = dataRode->header.pos("buyer-phone-number");
    int indSku = dataRode->header.pos("sku");
    int indProductName = dataRode->header.pos("product-name");
    int indQuantityShipped = dataRode->header.pos("quantity-purchased");
    int indCurrency = dataRode->header.pos("currency");
    int indItemPrice = dataRode->header.pos("item-price");
    int indItemTax = dataRode->header.pos("item-tax");
    int indShippingPrice = dataRode->header.pos("shipping-price");
    int indShippingTax = dataRode->header.pos("shipping-tax");
    int indItemPromo = dataRode->header.pos("item-promotion-discount"); ///tax free
    int indItemShippingPromo = dataRode->header.pos("ship-promotion-discount"); ///tax free I THINK
    int indShipAddress1 = dataRode->header.pos("ship-address-1");
    int indShipAddress2 = dataRode->header.pos("ship-address-2");
    int indShipAddress3 = dataRode->header.pos("ship-address-3");
    int indShipCity = dataRode->header.pos("ship-city");
    int indShipPostalCode = dataRode->header.pos("ship-postal-code");
    int indShipState = dataRode->header.pos("ship-state");
    int indShipCountry = dataRode->header.pos("ship-country");
    int indIsBusinessOrder = dataRode->header.pos("is-business-order");
    int indSubchannel = dataRode->header.pos("sales-channel");
    /*
    QString errorColumnText = QObject::tr("Il faut ajouter les colonnes «Identificateurs d'impôt de l'acheteur» et «Nom de la société de l'acheteur» dans Commandes > Rapport sur les commandes > Ajouter ou supprimer des colonnes de rapport sur la commande");
    int indBuyerTaxNumber = dataRode->header.pos(
                "buyer-tax-registration-id",
                errorColumnText);
    int indBuyerCompanyName = dataRode->header.pos(
                "buyer-company-name",
                errorColumnText);
                //*/

    //TODO handle 2 types of header…and check for csv
    // => Create class to manage efficiently different header versions
    for (auto elements : dataRode->lines) {
        QDateTime dateTimeOrder = dateTimeFromString(elements[indPurchaseDate]);
        if (dateTimeOrder.date().year() > maxYear) {
            continue;
        }
        orderMapping->minDate = qMin(dateTimeOrder, orderMapping->minDate);
        orderMapping->maxDate = qMax(dateTimeOrder, orderMapping->maxDate);
        QString orderId = elements[indAmazonOrderId];
        QString currency = elements[indCurrency];
        QString subchannel = elements[indSubchannel];
        Address addressTo(elements[indBuyerName],
                          elements[indShipAddress1],
                          elements[indShipAddress2],
                          elements[indShipAddress3],
                          elements[indShipCity],
                          elements[indShipPostalCode],
                          elements[indShipCountry],
                          elements[indShipState],
                          elements[indBuyerPhone]);
        Address addressBillingTo = addressTo;
        Shipping shippingExtra(0., 0., currency);
        QHash<QString, QSharedPointer<ArticleSold>> articles;
        double shippingPriceUntaxed = elements[indShippingPrice].toDouble();
        double shippingTaxes = elements[indShippingTax].toDouble();
        double shippingPricePromoUntaxed = elements[indItemShippingPromo].toDouble();
        double shippingPricePromoTaxes = shippingTaxes * shippingPricePromoUntaxed / shippingPriceUntaxed;
        double shippingTotalUntaxed = shippingPriceUntaxed + shippingPricePromoUntaxed;
        double shippingTotalTaxes = shippingTaxes + shippingPricePromoTaxes;
        /*
        if (shippingTotalTaxes > 0) {
            Q_ASSERT(false);
        }
        //*/
        shippingTotalTaxes = 0.; // Because amazon remove it TAX TODO double check
        Shipping shippingArticle(
                    shippingTotalUntaxed + shippingTotalTaxes,
                    shippingTotalTaxes,
                    currency);
        double articlePriceUntaxed = elements[indItemPrice].toDouble();
        double articleTaxes = elements[indItemTax].toDouble();
        double articlePricePromoUntaxed = elements[indItemPromo].toDouble();
        double articlePricePromoTaxes = articleTaxes * articlePricePromoUntaxed / articlePriceUntaxed;
        double totalTaxes = articleTaxes + articlePricePromoTaxes;
        double totalPriceTaxed = articlePriceUntaxed + articlePricePromoUntaxed;
        int quantityShipped = elements[indQuantityShipped].toInt();
        QString articleShipmentId = orderId + "-" + elements[indSku]; /// Only usefull for amazon FBA orders
        QSharedPointer<ArticleSold> article(
                    new ArticleSold(
                        articleShipmentId,
                        elements[indSku],
                        elements[indProductName],
                        ManagerSaleTypes::SALE_PRODUCTS,
                        quantityShipped,
                        totalPriceTaxed, // TAX
                        totalTaxes, // TAX
                        currency));
        article->setShipping(shippingArticle);
        articles[article->getShipmentItemId()] = article;
        //fbaCenterCode
        QString addressId = getDefaultShippingAddressId();;
        Address addressFrom = ShippingAddressesManager::instance()->getAddress(addressId);
        QDateTime dateTimeShipping = dateTimeOrder; /// To make things easier, we do as order were shipped same day
        QString shipmentId = orderId;  /// As shipment ID we put order ID assuming all orders will be shipped in once which could be wrong sometimes
        QSharedPointer<Shipment> shipment(
                    new Shipment(shipmentId,
                                 articles,
                                 shippingArticle,
                                 dateTimeShipping,
                                 addressFrom,
                                 currency,
                                 totalTaxes + shippingTotalTaxes));
                                 // 0.)); // TAX
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
        bool isBusinessCustomer = elements[indIsBusinessOrder].toLower() == "true";
        order->setIsBusinessCustomer(isBusinessCustomer);
        if (isVatToRecompute()) {
            order->setVatToRecompute(true);
        }
        /*
        QString vatNumber = elements[indBuyerTaxNumber];
        order->setVatNumber(vatNumber);
        QString companyName = elements[indBuyerCompanyName];
        order->setCompanyName(companyName);
        //*/

        int yearShipment = dateTimeShipping.date().year(); /// Amazon use shippig date for invoicne / vat report
        if (orderId == "111-7562514-2177011") {
            int TEMP=10;++TEMP;
        }
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
            orderMapping->shipmentById[shipmentId]->merge(*shipment.data()); // should not be needed as done by order->merge
        } else {
            orderMapping->shipmentById[shipmentId] = shipment;
            orderMapping->shipmentByDate[yearShipment].insert(dateTimeShipping, shipment);
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
QSharedPointer<OrdersMapping> OrderImporterAmazon::_loadReportPayments(
        const QString &fileName, int maxYear) const
{
    QSharedPointer<OrdersMapping> orderMapping(new OrdersMapping);
    orderMapping->addYears(12);
    CsvReader reader = createAmazonReader(fileName);
    reader.readAll();
    auto firstLineElements = reader.takeFirstLine();
    const DataFromCsv *dataRode = reader.dataRode();
    int indSettlementId = dataRode->header.pos("settlement-id");
    int indCurrency = dataRode->header.pos("currency");
    QString currency = firstLineElements[indCurrency];
    QString settlementId = firstLineElements[indSettlementId];
    int indStartDate = dataRode->header.pos("settlement-start-date");
    int indEndDate = dataRode->header.pos("settlement-end-date");
    orderMapping->minDate = dateTimeFromString(firstLineElements[indStartDate]);
    orderMapping->maxDate = dateTimeFromString(firstLineElements[indEndDate]);

    int indTransactionType = dataRode->header.pos("transaction-type");
    int indAmazonOrderId = dataRode->header.pos("order-id");
    int indRefundId = dataRode->header.pos("adjustment-id");
    int indMerhantOrderId = dataRode->header.pos("merchant-order-id");
    int indShipmentId = dataRode->header.pos("shipment-id");
    int indSubchannel = dataRode->header.pos("marketplace-name");
    int indAmountType = dataRode->header.pos("amount-type");
    int indAmountDescription = dataRode->header.pos("amount-description");
    int indAmount = dataRode->header.pos("amount");
    int indShippingDate = dataRode->header.pos("posted-date-time");
    int indSku = dataRode->header.pos("sku");
    QHash<QString, QString> refundSubchannels;
    //int indQuantityPurchased = dataRode->header.pos("quantity-purchased");

    int numLine = 1;
    /// order id          refund-id      refunddate        sku            amount-type     amount-desc
    QHash<QString, QHash<QString, QHash<QDateTime, QHash<QString, QHash<QString, QMultiHash<QString, double>>>>>> refundInfos;
    for (auto elements : dataRode->lines) {
        ++numLine;
        QString transactionType = elements[indTransactionType];
        QString merchentOrderId = elements[indMerhantOrderId];
        QString orderId = elements[indAmazonOrderId];
        QString subchannel = elements[indSubchannel];
        QString amountString = elements[indAmount].replace(",", ".");
        double amount = amountString.toDouble();
        //double amount1 = QString("1.12").toDouble();
        //double amount2 = QString("-1.12").toDouble();
        QString amountType = elements[indAmountType];
        QString amountDescription = elements[indAmountDescription];
        QDateTime dateTimeShipping = dateTimeFromString(elements[indShippingDate]);
        if (dateTimeShipping.date().year() > maxYear) {
            continue;
        }
        if (orderId == "113-9168509-7269803") {
                //|| orderId == "113-6996273-9724251") {
            int TEMP=10;++TEMP;
        }
        //orderMapping->minDate = qMin(dateTimeShipping, orderMapping->minDate);
        //orderMapping->maxDate = qMax(dateTimeShipping, orderMapping->maxDate);
        QString sku = elements[indSku];
        //int quantity = elements[indQuantityPurchased].toInt();
        // order id          shippingDate     sku            amount-type    amount-desc    trans-id
        //QHash<QString, QHash<QDateTime, QHash<QString, QHash<QString, QHash<QString, QHash<QString, double>>>>>> refundInfos;
        if (transactionType.toLower() == "order") {
            if (subchannel.contains("mazon.")) {
                QString shipmentId = elements[indShipmentId];
                if (merchentOrderId.isEmpty()) {
                    shipmentId = orderId;
                }
                QHash<QString, QSharedPointer<ArticleSold>> articles;
                Address addressUnknown;
                QSharedPointer<Shipment> shipment(
                            new Shipment(shipmentId,
                                         articles,
                                         Shipping(0., 0., currency),
                                         dateTimeShipping, ///Could be wrong some times
                                         addressUnknown,
                                         currency,
                                         0.));
                shipment->setPaymentId(settlementId);
                QString feeId = settlementId + "-" + QString::number(numLine);
                shipment->addChargedFee(feeId, amountDescription, sku, amount);
                QHash<QString, QSharedPointer<Shipment>> shipments;
                shipments[shipment->getId()] = shipment;
                QSharedPointer<Order> order(
                            new Order(
                                QDateTime(),
                                orderId,
                                currency,
                                name(),
                                subchannel,
                                addressUnknown,
                                addressUnknown,
                                Shipping(0., 0., currency),
                                true,
                                shipments));
                bool isBusinessCustomer = false; //TODO find how to guess this
                order->setIsBusinessCustomer(isBusinessCustomer);
                if (isVatToRecompute()) {
                    order->setVatToRecompute(true);
                }
                if (orderId == "113-8195834-2026639") {
                    int TEMP=10;++TEMP;
                }
                int yearShipment = dateTimeShipping.date().year(); /// Amazon use shippig date for invoicne / vat report
                if (orderMapping->orderById.contains(orderId)) {
                    orderMapping->orderById[orderId]->merge(*order.data());
                } else {
                    orderMapping->orderById[orderId] = order;
                    if (!orderMapping->ordersQuantityByDate[yearShipment].contains(order->getSubchannel())) {
                        orderMapping->ordersQuantityByDate[yearShipment][order->getSubchannel()] = QMap<QDate, int>();
                    }
                    if (!orderMapping->ordersQuantityByDate[yearShipment][order->getSubchannel()].contains(dateTimeShipping.date())) {
                        orderMapping->ordersQuantityByDate[yearShipment][order->getSubchannel()][dateTimeShipping.date()] = 1;
                    } else {
                        orderMapping->ordersQuantityByDate[yearShipment][order->getSubchannel()][dateTimeShipping.date()] += 1;
                    }
                }
                if (orderMapping->shipmentById.contains(shipmentId)) {
                    orderMapping->shipmentById[shipmentId]->merge(*shipment.data()); // should not be needed as done by order->merge
                } else {
                    orderMapping->shipmentById[shipmentId] = shipment;
                    orderMapping->shipmentByDate[yearShipment].insert(dateTimeShipping, shipment);
                }
            }
        } else if (transactionType.toLower() == "refund") {
            //QString refundId = orderId + dateTimeShipping.toString("_yyyy-MM-dd_hh-mm-ss"); // + "_" + QString::number(amount, 'f', 3);
            QString refundId = elements[indRefundId];
            refundSubchannels[refundId] = subchannel;
            if (!refundInfos.contains(orderId)) {
                refundInfos[orderId] = QHash<QString, QHash<QDateTime, QHash<QString, QHash<QString, QMultiHash<QString, double>>>>>();
            }
            if (!refundInfos[orderId].contains(refundId)) {
                refundInfos[orderId][refundId] = QHash<QDateTime, QHash<QString, QHash<QString, QMultiHash<QString, double>>>>();
            }
            if (!refundInfos[orderId][refundId].contains(dateTimeShipping)) {
                refundInfos[orderId][refundId][dateTimeShipping] = QHash<QString, QHash<QString, QMultiHash<QString, double>>>();
            }
            if (!refundInfos[orderId][refundId][dateTimeShipping].contains(sku)) {
                refundInfos[orderId][refundId][dateTimeShipping][sku] = QHash<QString, QMultiHash<QString, double>>();
            }
            if (!refundInfos[orderId][refundId][dateTimeShipping][sku].contains(amountType)) {
                refundInfos[orderId][refundId][dateTimeShipping][sku][amountType] = QMultiHash<QString, double>();
            }
            refundInfos[orderId][refundId][dateTimeShipping][sku][amountType].insert(amountDescription, amount);
        } else if (!transactionType.isEmpty()) {
            int year = dateTimeShipping.date().year();
            if (!orderMapping->nonOrderFees[year].contains(settlementId)) {
                orderMapping->nonOrderFees[year][settlementId]
                        = QMap<QDateTime, QMultiMap<QString, double>>();
            }
            if (!orderMapping->nonOrderFees[year][settlementId].contains(dateTimeShipping)) {
                orderMapping->nonOrderFees[year][settlementId][dateTimeShipping] = QMultiMap<QString, double>();
            }
            orderMapping->nonOrderFees[year][settlementId][dateTimeShipping].insert(amountDescription, amount);
            FeesTableModel::instance(name())->addFees(amountDescription);
        }
    }
             /// order id          refund-id      refunddate        sku            amount-type     amount-desc
            //QHash<QString, QHash<QString, QHash<QDateTime, QHash<QString, QHash<QString, QHash<QString, double>>>>>> refundInfos;
    for (auto itOrder = refundInfos.begin();
         itOrder != refundInfos.end();
         ++itOrder) {
        for (auto itRefundId = itOrder.value().begin();
             itRefundId != itOrder.value().end();
             ++itRefundId) {
            for (auto itDate = itRefundId.value().begin();
                 itDate != itRefundId.value().end();
                 ++itDate) {
                QHash<QString, QSharedPointer<ArticleSold>> articlesSold;
                double vatForRoundCorrection = 0.;
                for (auto itSku = itDate.value().begin();
                     itSku != itDate.value().end();
                     ++itSku) {
                    double amountTaxed = 0.;
                    double amountTaxes = 0.;
                    double amazonFees = 0.;
                    int quantity = 0;
                    for (auto itAmountType = itSku.value().begin();
                         itAmountType != itSku.value().end();
                         ++itAmountType) {
                        for (auto itAmountDesc = itAmountType.value().begin();
                             itAmountDesc != itAmountType.value().end();
                             ++itAmountDesc) {
                            if (itAmountDesc.key().contains("Principal")) {
                                quantity +=1;
                            }
                            if (itAmountType.key().contains("ItemPrice")
                                    || itAmountType.key().contains("ItemWithheldTax")
                                    || itAmountType.key().contains("Promotion")) {
                                if (!itAmountDesc.key().contains("Tax")) {
                                    amountTaxed += itAmountDesc.value();
                                }
                                if (itAmountDesc.key().contains("Tax")) {
                                    /* // Tax to removed
                                    amountTaxes += itAmountDesc.value();
                                    vatForRoundCorrection += itAmountDesc.value();;
                                    //*/
                                }
                            } else if (itAmountType.key().contains("ItemFees")) {
                                amazonFees += itAmountDesc.value();
                            } else {
                                QString orderId = itOrder.key();
                                QString amountDesc = itAmountDesc.key();
                                QString amountType = itAmountType.key();
                                Q_ASSERT(false);
                            }
                        }
                    }
                    QString articleId = itSku.key() + "__" + itRefundId.key();
                    if (itRefundId.key() == "amzn1:crow:C8PV6JL8Q0m0VrBLvCOFzw") {
                        int TEMP=10;++TEMP;
                    }
                    if (articleId == "Dn8pPl9vR") {
                        int TEMP=10;++TEMP;
                    }
                    auto articleRefunded = QSharedPointer<ArticleSold>(
                                new ArticleSold(itRefundId.key(),
                                                itSku.key(),
                                                "", //TODO title
                                                ManagerSaleTypes::SALE_PRODUCTS,
                                                quantity,
                                                amountTaxed,
                                                amountTaxes,
                                                currency));
                    articlesSold[articleId] = articleRefunded;
                }
                QString refundId = itRefundId.key();
                if (!orderMapping->refundById.contains(refundId)) {
                    QSharedPointer<Refund> refund(
                                new Refund(itRefundId.key(),
                                           itOrder.key(),
                                           articlesSold,
                                           Shipping(0., 0., currency),
                                           itDate.key(),
                                           Address(),
                                           currency,
                                           vatForRoundCorrection));
                    refund->setChannel(name());
                    refund->setPaymentId(settlementId);
                    refund->setSubchannel(refundSubchannels[refundId]);
                    // TODO refund->addChargedFee(-1, "Commission", itSku.key(), amazonFees);
                    int year = itDate.key().date().year();
                    orderMapping->refundById[refundId] = refund;
                    orderMapping->refundByOrderId.insert(itOrder.key(), refund);
                    orderMapping->refundByDate[year].insert(itDate.key(), refund);
                }
            }
        }
    }
    for (auto refund : orderMapping->refundById) {
        QString orderId = refund->orderId();
        if (orderMapping->orderById.contains(orderId)) {
            auto order = orderMapping->orderById[orderId];
            refund->init(order.data());
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
QSharedPointer<OrdersMapping> OrderImporterAmazon::_loadReportInvoicing(
        const QString &fileName, int maxYear) const
{
    QString relFileName = QFileInfo(fileName).fileName();
    QSharedPointer<OrdersMapping> orderMapping(new OrdersMapping);
    orderMapping->addYears(12);
    CsvReader reader = createAmazonReader(fileName);
    reader.readAll();
    const DataFromCsv *dataRode = reader.dataRode();

    int indAmazonOrderId = dataRode->header.pos({"order-id", "amazon-order-id", "Amazon Order Id"});
    int indMerchentOrderId = dataRode->header.pos({"Merchant Order Id", "merchant-order-id", "Merchant Order ID"});
    int indShipmentId = dataRode->header.pos({"shipment-id", "Shipment ID", "Shipment Id"});
    int indArticleShipmentId = dataRode->header.pos({"shipment-item-id", "Shipment Item ID", "Shipment Item Id"});
    int indPurchaseDate = dataRode->header.pos({"purchase-date", "Purchase Date"});
    int indShippingDate = dataRode->header.pos({"shipment-date", "Shipment Date"});
    int indCurrency = dataRode->header.pos({"currency", "Currency"});
    int indSubchannel = dataRode->header.pos({"sales-channel", "Sales Channel"});
    int indSku = dataRode->header.pos({"sku", "Merchant SKU"});
    int indProductName = dataRode->header.pos({"product-name", "Title"});
    int indQuantityShipped = dataRode->header.pos({"Shipped Quantity", "quantity-shipped", "Dispatched Quantity"});
    int indItemPriceUntaxed = dataRode->header.pos({"item-price", "Item Price"});
    int indItemTax = dataRode->header.pos({"item-tax", "Item Tax"});
    int indShippingPrice = dataRode->header.pos({"Shipping Price", "shipping-price", "Delivery Price"});
    int indShippingTax = dataRode->header.pos({"Shipping Tax", "shipping-tax", "Delivery Tax"});
    int indGiftPrice = dataRode->header.pos({"gift-wrap-price", "Gift Wrap Price"});
    int indGiftTax = dataRode->header.pos({"Gift Wrap Tax", "gift-wrap-tax", "Gift Wrapping Tax"});
    int indItemPromo = dataRode->header.pos({"item-promotion-discount", "Item Promo Discount"}); ///tax free
    int indItemShippingPromo = dataRode->header.pos({"ship-promotion-discount", "Shipment Promo Discount"}); ///tax free I THINK
    int indBuyerName = dataRode->header.pos({"recipient-name", "Recipient Name"});
    int indBillAddress1 = dataRode->header.pos({"bill-address-1", "Billing Address 1"});
    int indBillAddress2 = dataRode->header.pos({"bill-address-2", "Billing Address 2"});
    int indBillAddress3 = dataRode->header.pos({"bill-address-3", "Billing Address 3"});
    int indBillCity = dataRode->header.pos({"Billing City", "bill-city", "Billing City/Town"});
    int indBillPostalCode = dataRode->header.pos({"bill-postal-code", "Billing Postal Code"});
    int indBillState = dataRode->header.pos({"Billing State", "bill-state", "Billing County"});
    int indBillCountry = dataRode->header.pos({"bill-country", "Billing Country Code"});
    int indShipAddress1 = dataRode->header.pos({"Shipping Address 1", "ship-address-1", "Delivery Address 1"});
    int indShipAddress2 = dataRode->header.pos({"Shipping Address 1", "ship-address-2", "Delivery Address 2"});
    int indShipAddress3 = dataRode->header.pos({"Shipping Address 1", "ship-address-3", "Delivery Address 3"});
    int indShipCity = dataRode->header.pos({"Shipping City", "ship-city", "Delivery City/Town"});
    int indShipPostalCode = dataRode->header.pos({"Shipping Postal Code", "ship-postal-code", "Delivery Postcode"});
    int indShipState = dataRode->header.pos({"Shipping State", "ship-state", "Delivery County"});
    int indShipCountry = dataRode->header.pos({"Shipping Country Code", "ship-country", "Delivery Country Code"});
    int indBuyerPhone = dataRode->header.pos({"Shipping Phone Number", "ship-phone-number", "Delivery Phone Number"});
    int indFbaCenter = dataRode->header.pos({"fulfillment-center-id", "FC"});

    //TODO handle 2 types of header…and check for csv
    // => Create class to manage efficiently different header versions
    for (auto elements : dataRode->lines) {
        QString merchantOrderId = elements[indMerchentOrderId];
        if (!merchantOrderId.isEmpty()) {
            continue; //TODO save information somewhere?
        }
        QDateTime dateTimeOrder = dateTimeFromString(elements[indPurchaseDate]);
        if (dateTimeOrder.date().year() > maxYear) {
            continue;
        }
        orderMapping->minDate = qMin(dateTimeOrder, orderMapping->minDate);
        QString orderId = elements[indAmazonOrderId];
        if (orderId == "113-9168509-7269803") {
                //|| orderId == "113-6996273-9724251") {
            int TEMP=10;++TEMP;
        }
        QString shipmentId = elements[indShipmentId];
        QString articleShipmentId = elements[indArticleShipmentId];
        QString currency = elements[indCurrency];
        QString subchannel = elements[indSubchannel];
        if (!subchannel.contains("mazon.")) {
            continue;
        }
        QString countryCodeTo = CountryManager::instance()->countryCodeDomTom(
                    elements[indShipCountry], elements[indShipPostalCode]);
        QString countryBillingCodeTo = CountryManager::instance()->countryCodeDomTom(
                    elements[indBillCountry], elements[indBillPostalCode]);
        Address addressTo(elements[indBuyerName],
                          elements[indShipAddress1],
                          elements[indShipAddress2],
                          elements[indShipAddress3],
                          elements[indShipCity],
                          elements[indShipPostalCode],
                          countryCodeTo,
                          elements[indShipState],
                          elements[indBuyerPhone]);
        Address addressBillingTo;
        if (countryBillingCodeTo.isEmpty()) {
            addressBillingTo = addressTo;
        } else {
            addressBillingTo = Address(elements[indBuyerName],
                                       elements[indBillAddress1],
                                       elements[indBillAddress2],
                                       elements[indBillAddress3],
                                       elements[indBillCity],
                                       elements[indBillPostalCode],
                                       countryBillingCodeTo,
                                       elements[indBillState],
                                       elements[indBuyerPhone]);
        }
        if (countryCodeTo.contains("-") || countryCodeTo.isEmpty()) {
            if (!countryBillingCodeTo.contains("-") && !countryBillingCodeTo.isEmpty()) {
                addressTo = addressBillingTo;
            }
        }


        Shipping shippingExtra(0., 0., currency);
        QHash<QString, QSharedPointer<ArticleSold>> articles;
        double shippingPriceUntaxed = elements[indShippingPrice].toDouble();
        double shippingTaxes = elements[indShippingTax].toDouble();
        double shippingPricePromoUntaxed = elements[indItemShippingPromo].toDouble();
        double shippingPricePromoTaxes = 0.;
        if (shippingPriceUntaxed > 0.) {
            shippingPricePromoTaxes = shippingTaxes * shippingPricePromoUntaxed / shippingPriceUntaxed;
        }
        double giftPriceUntaxed = elements[indGiftPrice].toDouble();
        double giftTaxes = elements[indGiftTax].toDouble();
        double shippingTotalUntaxed = shippingPriceUntaxed + giftPriceUntaxed + shippingPricePromoUntaxed;
        double shippingTotalTaxes = shippingTaxes + giftTaxes + shippingPricePromoTaxes;

        double articlePriceUntaxed = elements[indItemPriceUntaxed].toDouble();
        if (qAbs(articlePriceUntaxed) < 0.001) {
            continue; /// It means it is a replacement not paid
        }
        double articleTaxes = elements[indItemTax].toDouble();
        double articlePricePromoUntaxed = elements[indItemPromo].toDouble();
        double articlePricePromoTaxes = articleTaxes * articlePricePromoUntaxed / articlePriceUntaxed;
        double totalTaxes = articleTaxes + articlePricePromoTaxes;
        QDateTime dateTimeShipping = dateTimeFromString(elements[indShippingDate]);
        //* /// TODO find why I removed?
        if (orderId == "113-8195834-2026639") {
            int TEMP=10;++TEMP;
        }
        //if (SettingManager::instance()->countriesUE(
                    //dateTimeShipping.date().year())->contains(countryCodeTo)) {
            shippingTotalTaxes = 0.; /// Because of marketplace facilitor
            articleTaxes = 0.;
            totalTaxes = 0.;
        //}
        //*/
        double totalPriceTaxed = articlePriceUntaxed + articlePricePromoUntaxed + totalTaxes;
        int quantityShipped = elements[indQuantityShipped].toInt();
        //*
        Shipping shippingArticle(
                    shippingTotalUntaxed + shippingTotalTaxes,
                    shippingTotalTaxes,
                    currency);
                    //*/
        QSharedPointer<ArticleSold> article(
                    new ArticleSold(
                        elements[indArticleShipmentId],
                        elements[indSku],
                        elements[indProductName],
                        ManagerSaleTypes::SALE_PRODUCTS,
                        quantityShipped, //TODO find order with same item shipped in twice to see if two line with different shipping date
                        totalPriceTaxed - totalTaxes,
                        totalTaxes - totalTaxes,
                        elements[indCurrency]));
        article->setShipping(shippingArticle);
        QString articleId = article->getShipmentItemId();
        articles[articleId] = article;
        QString fbaCenterCode = elements[indFbaCenter];
        Address addressFrom = _addressFromSubchannel(subchannel, fbaCenterCode);
        if (shipmentId == "MkxjKdsf0") {
            int TEMP=10;++TEMP;
        }
        /*
        if (AmazonFulfillmentAddressModel::instance()->contains(fbaCenterCode)) {
            addressFrom = AmazonFulfillmentAddressModel::instance()->getAddress(fbaCenterCode);
        } else {
            OrderImporterException exception;
            exception.setError(QObject::tr("Le centre d'expédition FBA %1"
                                 " est inconnu (%2). Il faudrait trouver le pays puis"
                                 " l'ajouter dans la liste des centres d'expédition FBA."
                                 " Notez bien le code à ajouter avant de fermer cette fenêtre.")
                               .arg(fbaCenterCode, elements[indSubchannel]));
            exception.raise();
        }
        //*/
        if (elements[indShippingDate].contains("+")) { // Is there a - also for some countries ?
            QTime timeAdded = QTime::fromString(elements[indShippingDate].split("+").last(), "hh:mm");
            int secs = QTime(0, 0).secsTo(timeAdded);
            dateTimeShipping = dateTimeShipping.addSecs(-secs);
        }
        orderMapping->maxDate = qMax(dateTimeShipping, orderMapping->maxDate);
        QSharedPointer<Shipment> shipment(
                    new Shipment(elements[indShipmentId],
                                 articles,
                                 Shipping(0., 0., currency),
                                 dateTimeShipping,
                                 addressFrom,
                                 currency,
                                 0.)); //totalTaxes + shippingTotalTaxes));
        //shipment->setVatCollectResponsible(
                    //Shipment::VAT_RESPONSIBLE_MARKETPLACE);
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
        order->setShippedBySeller(false);
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
            orderMapping->shipmentById[shipmentId]->merge(*shipment.data()); // should not be needed as done by order->merge
        } else {
            orderMapping->shipmentById[shipmentId] = shipment;
            orderMapping->shipmentByDate[yearShipment].insert(dateTimeShipping, shipment);
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
