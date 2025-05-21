#include <QtCore/QLocale>

#include "../common/countries/CountryManager.h"

#include "OrderImporterAmazonUE.h"
#include "model/vat/AmazonFulfillmentAddressModel.h"
#include "model/vat/ManagerCompanyVatParams.h"
#include "model/orderimporters/FeesTableModel.h"
#include "model/orderimporters/ShippingAddressesManager.h"
#include "model/SettingManager.h"
#include "model/bookkeeping/ManagerSaleTypes.h"

const QString OrderImporterAmazonUE::NAME = "Amazon UE";
const QString OrderImporterAmazonUE::REPORT_VAT
= QObject::tr("Rapport des transactions Amazon liées à la TVA", "Amazon report name to find in user interface");
const QString OrderImporterAmazonUE::REPORT_VAT_SHORT
= QObject::tr("Rapports TVA");
const QString OrderImporterAmazonUE::REPORT_ORDERS_FBM
= QObject::tr("Rapports sur les commandes", "Amazon report name to find in user interface");
const QString OrderImporterAmazonUE::REPORT_ORDERS_FBM_SHORT
= QObject::tr("Commandes FBM");
const QString OrderImporterAmazonUE::REPORT_ORDERS_PAYMENTS
= QObject::tr("Paiements", "Amazon report name to find in user interface");
const QString OrderImporterAmazonUE::REPORT_ORDERS_INVOICING
= QObject::tr("Expéditions effectuées par Amazon – Facturation des taxes", "Amazon report name to find in user interface");
const QString OrderImporterAmazonUE::REPORT_ORDERS_INVOICING_SHORT
= QObject::tr("Commandes FBA (facturation)");
//----------------------------------------------------------
OrderImporterAmazonUE::OrderImporterAmazonUE() : AbstractOrderImporter()
{
}
//----------------------------------------------------------
QString OrderImporterAmazonUE::name() const
{
    return NAME;
}
//----------------------------------------------------------
QString OrderImporterAmazonUE::invoicePrefix() const
{
    return "AMZEU";
}
//----------------------------------------------------------
QString OrderImporterAmazonUE::uniqueId() const
{
    return "OrderImporterAmazonUE";
}
//----------------------------------------------------------
QList<ReportType> OrderImporterAmazonUE::reportTypes() const
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
    ReportType typeVatReport;
    //typeVatReport.name = REPORT_VAT;
    typeVatReport.shortName = REPORT_VAT_SHORT;
    typeVatReport.extensions = QStringList({"csv", "txt"});
    typeVatReport.helpText = QObject::tr("Rapport > Expédié par amazon > Stock > Rapport des transactions Amazon liées à la TVA (csv)");
    reportTypes << typeVatReport;
    return reportTypes;
}
//----------------------------------------------------------
QSharedPointer<OrdersMapping> OrderImporterAmazonUE::loadReport(
        const QString &reportTypeName,
        const QString &fileName, int maxYear) const {
    QSharedPointer<OrdersMapping> orderMapping;
    if (reportTypeName == OrderImporterAmazonUE::REPORT_ORDERS_INVOICING
            || reportTypeName == OrderImporterAmazonUE::REPORT_ORDERS_INVOICING_SHORT) {
        orderMapping = _loadReportInvoicing(fileName, maxYear);
    } else if (reportTypeName == OrderImporterAmazonUE::REPORT_ORDERS_FBM
              || reportTypeName == OrderImporterAmazonUE::REPORT_ORDERS_FBM_SHORT) {
        orderMapping = _loadReportOrdersFbm(fileName, maxYear);
    } else if (reportTypeName == OrderImporterAmazonUE::REPORT_ORDERS_PAYMENTS) {
        orderMapping = _loadReportPayments(fileName, maxYear);
    } else if (reportTypeName == OrderImporterAmazonUE::REPORT_VAT
               || reportTypeName == OrderImporterAmazonUE::REPORT_VAT_SHORT) {
        orderMapping = _loadReportVat(fileName, maxYear);
    }
    for (auto itOrder = orderMapping->orderById.begin();
         itOrder != orderMapping->orderById.end(); ++itOrder) {
        updateOrderIfMicroCountry(itOrder->data());
        for (auto shipment : (*itOrder)->getShipments()) {
            shipment->addReportFrom(
                        orderMapping->maxDate.date(), reportTypeName);
        }
    }
    for (auto refund : orderMapping->refundById) {
        refund->addReportFrom(orderMapping->maxDate.date(), reportTypeName);
    }
    return orderMapping;
}
//----------------------------------------------------------
CsvReader OrderImporterAmazonUE::createAmazonReader(
        const QString &fileName)
{
    QString sep = "\t";
    QString guill = "";
    if (fileName.endsWith("csv")) {
        sep = ",";
        guill = "\"";
    }
    CsvReader reader(fileName, sep, guill);
    return reader;
}
//----------------------------------------------------------
QList<QStringList> OrderImporterAmazonUE::reportForOrderComplete(
        const Order *order) const
{
    static QList<QStringList> reports
            = {
        {REPORT_ORDERS_INVOICING_SHORT, REPORT_VAT_SHORT, REPORT_ORDERS_PAYMENTS}
        , {REPORT_ORDERS_INVOICING_SHORT, REPORT_VAT_SHORT}
        , {REPORT_ORDERS_FBM_SHORT}
        , {REPORT_ORDERS_FBM_SHORT, REPORT_ORDERS_PAYMENTS}
    };
    static QList<QStringList> reportsNotUE
            = {
        {REPORT_ORDERS_INVOICING_SHORT, REPORT_VAT_SHORT, REPORT_ORDERS_PAYMENTS}
        , {REPORT_ORDERS_INVOICING_SHORT, REPORT_ORDERS_PAYMENTS}
        , {REPORT_ORDERS_INVOICING_SHORT, REPORT_VAT_SHORT}
        , {REPORT_ORDERS_FBM_SHORT}
        , {REPORT_ORDERS_FBM_SHORT, REPORT_ORDERS_PAYMENTS}
    };
    if (!CountryManager::instance()->countriesCodeUEfrom2020()->contains(
                order->getAddressTo().countryCode())) {
        return reportsNotUE;
    }
    return reports;
}
//----------------------------------------------------------
QSharedPointer<OrdersMapping> OrderImporterAmazonUE::_loadReportInvoicing(const QString &fileName, int maxYear) const
{
    QString relFileName = QFileInfo(fileName).fileName();
    QSharedPointer<OrdersMapping> orderMapping(new OrdersMapping);
    orderMapping->addYears(12);
    CsvReader reader = createAmazonReader(fileName);
    reader.readAll();
    const DataFromCsv *dataRode = reader.dataRode();

    int indAmazonOrderId = dataRode->header.pos({"order-id", "amazon-order-id", "Amazon Order Id"});
    int indMerchentOrderId = dataRode->header.pos({"merchant-order-id", "Merchant Order ID", "Merchant Order Id"});
    int indShipmentId = dataRode->header.pos({"shipment-id", "Shipment ID", "Shipment Id"});
    int indArticleShipmentId = dataRode->header.pos({"shipment-item-id", "Shipment Item ID", "Shipment Item Id"});
    int indArticleOrderId = dataRode->header.pos({"shipment-order-item-id", "Amazon Order Item ID", "Amazon Order Item Id"});
    int indPurchaseDate = dataRode->header.pos({"purchase-date", "Purchase Date"});
    int indShippingDate = dataRode->header.pos({"shipment-date", "Shipment Date"});
    int indCurrency = dataRode->header.pos({"currency", "Currency"});
    int indSubchannel = dataRode->header.pos({"sales-channel", "Sales Channel"});
    int indSku = dataRode->header.pos({"sku", "Merchant SKU"});
    int indProductName = dataRode->header.pos({"product-name", "Title"});
    int indQuantityShipped = dataRode->header.pos({"quantity-shipped", "Dispatched Quantity", "Shipped Quantity"});
    int indItemPriceUntaxed = dataRode->header.pos({"item-price", "Item Price"});
    int indItemTax = dataRode->header.pos({"item-tax", "Item Tax"});
    int indShippingPrice = dataRode->header.pos({"shipping-price", "Delivery Price", "Shipping Price"});
    int indShippingTax = dataRode->header.pos({"shipping-tax", "Delivery Tax", "Shipping Tax"});
    int indGiftPrice = dataRode->header.pos({"gift-wrap-price", "Gift Wrap Price"});
    int indGiftTax = dataRode->header.pos({"gift-wrap-tax", "Gift Wrapping Tax", "Gift Wrap Tax"});
    int indItemPromo = dataRode->header.pos({"item-promotion-discount", "Item Promo Discount"}); ///tax free
    int indItemShippingPromo = dataRode->header.pos({"ship-promotion-discount", "Shipment Promo Discount"}); ///tax free I THINK
    int indBuyerName = dataRode->header.pos({"recipient-name", "Recipient Name"});
    int indBillAddress1 = dataRode->header.pos({"bill-address-1", "Billing Address 1"});
    int indBillAddress2 = dataRode->header.pos({"bill-address-2", "Billing Address 2"});
    int indBillAddress3 = dataRode->header.pos({"bill-address-3", "Billing Address 3"});
    int indBillCity = dataRode->header.pos({"bill-city", "Billing City/Town", "Billing City"});
    int indBillPostalCode = dataRode->header.pos({"bill-postal-code", "Billing Postal Code"});
    int indBillState = dataRode->header.pos({"bill-state", "Billing County", "Billing State"});
    int indBillCountry = dataRode->header.pos({"bill-country", "Billing Country Code"});
    int indShipAddress1 = dataRode->header.pos({"ship-address-1", "Delivery Address 1", "Shipping Address 1"});
    int indShipAddress2 = dataRode->header.pos({"ship-address-2", "Delivery Address 2", "Shipping Address 2"});
    int indShipAddress3 = dataRode->header.pos({"ship-address-3", "Delivery Address 3", "Shipping Address 3"});
    int indShipCity = dataRode->header.pos({"ship-city", "Delivery City/Town", "Shipping City"});
    int indShipPostalCode = dataRode->header.pos({"ship-postal-code", "Delivery Postcode", "Shipping Postal Code"});
    int indShipState = dataRode->header.pos({"ship-state", "Delivery County", "Shipping State"});
    int indShipCountry = dataRode->header.pos({"ship-country", "Delivery Country Code", "Shipping Country Code"});
    int indBuyerPhone = dataRode->header.pos({"ship-phone-number", "Delivery Phone Number", "Shipping Phone Number"});
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
        if (orderId == "405-5686469-6205107") {
            int TEMP=10;++TEMP;
        }
        QString shipmentId = elements[indShipmentId];
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
        Address addressBillingTo(elements[indBuyerName],
                          elements[indBillAddress1],
                          elements[indBillAddress2],
                          elements[indBillAddress3],
                          elements[indBillCity],
                          elements[indBillPostalCode],
                          countryBillingCodeTo,
                          elements[indBillState],
                          elements[indBuyerPhone]);
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
        double articleTaxes = elements[indItemTax].toDouble();
        double articlePricePromoUntaxed = elements[indItemPromo].toDouble();
        double articlePricePromoTaxes = articleTaxes * articlePricePromoUntaxed / articlePriceUntaxed;
        double totalTaxes = articleTaxes + articlePricePromoTaxes;
        QDateTime dateTimeShipping = dateTimeFromString(elements[indShippingDate]);
        /*
        if (!SettingManager::instance()->countriesUE(
                    dateTimeShipping.date().year())->contains(countryCodeTo)) {
            shippingTotalTaxes = 0.; /// Because of marketplace facilitor
            articleTaxes = 0.;
            totalTaxes = 0.;
        }
        //*/
        double totalPriceTaxed = articlePriceUntaxed + articlePricePromoUntaxed + totalTaxes;
        int quantityShipped = elements[indQuantityShipped].toInt();
        Shipping shippingArticle(
                    shippingTotalUntaxed + shippingTotalTaxes,
                    shippingTotalTaxes,
                    currency);
        QString articleShipmentId = elements[indArticleShipmentId];
        QString articleOrderItemId = elements[indArticleOrderId]; // Due to a technical issue, one report can have this empty and not the next one
        if (articleShipmentId.isEmpty()) {
            //continue; // Not complete information
            articleShipmentId = articleOrderItemId;
        }
        QSharedPointer<ArticleSold> article(
                    new ArticleSold(
                        articleShipmentId,
                        elements[indSku],
                        elements[indProductName],
                        ManagerSaleTypes::SALE_PRODUCTS,
                        quantityShipped, //TODO find order with same item shipped in twice to see if two line with different shipping date
                        totalPriceTaxed,
                        totalTaxes,
                        elements[indCurrency]));
        article->setShipping(shippingArticle);
        QString articleId = article->getShipmentItemId();
        articles[articleId] = article;
        Address addressFrom;
        QString fbaCenterCode = elements[indFbaCenter];
        if (AmazonFulfillmentAddressModel::instance()->contains(fbaCenterCode)) {
            addressFrom = AmazonFulfillmentAddressModel::instance()->getAddress(fbaCenterCode);
        } else {
            OrderImporterException exception;
            exception.setError(QObject::tr("Le centre d'expédition FBA %1"
                                 " est inconnu (%2, %3). Il faudrait trouver le pays puis"
                                 " l'ajouter dans la liste des centres d'expédition FBA."
                                 " Notez bien le code à ajouter avant de fermer cette fenêtre.")
                               .arg(fbaCenterCode, elements[indSubchannel], orderId));
            exception.raise();
        }
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
                                 totalTaxes + shippingTotalTaxes));
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
QSharedPointer<OrdersMapping> OrderImporterAmazonUE::_loadReportVat(
        const QString &fileName, int maxYear) const
{
    QSharedPointer<OrdersMapping> orderMapping(new OrdersMapping);
    CsvReader reader = createAmazonReader(fileName);
    reader.readAll();
    const DataFromCsv *dataRode = reader.dataRode();
    int indTransactionType = dataRode->header.pos("TRANSACTION_TYPE");
    int indAmazonOrderId = dataRode->header.pos("TRANSACTION_EVENT_ID");
    int indSaleChannel = dataRode->header.pos("SALES_CHANNEL");
    int indBuyerVatNumber = dataRode->header.pos("BUYER_VAT_NUMBER");
    int indCurrency = dataRode->header.pos("TRANSACTION_CURRENCY_CODE");
    int indQty = dataRode->header.pos("QTY");
    int indSubchannel = dataRode->header.pos("MARKETPLACE");
    int indDestCity = dataRode->header.pos("ARRIVAL_CITY");
    int indDestPostcode = dataRode->header.pos("ARRIVAL_POST_CODE");
    int indDestCountry = dataRode->header.pos("ARRIVAL_COUNTRY");
    int indFromPostcode = dataRode->header.pos("DEPARTURE_POST_CODE");
    int indFromCity = dataRode->header.pos({"DEPATURE_CITY", "DEPARTURE_CiTY"});
    int indFromCountry = dataRode->header.pos("DEPARTURE_COUNTRY");
    int indDateShipping = dataRode->header.pos("TRANSACTION_COMPLETE_DATE");
    int indSKU = dataRode->header.pos("SELLER_SKU");
    int indRefundOrShipmentId = dataRode->header.pos("ACTIVITY_TRANSACTION_ID");
    int indItemName = dataRode->header.pos("ITEM_DESCRIPTION");
    int indAmountTotal = dataRode->header.pos("TOTAL_ACTIVITY_VALUE_AMT_VAT_INCL");
    int indAmountTaxes = dataRode->header.pos("TOTAL_ACTIVITY_VALUE_VAT_AMT");
    int indVatInvoiceNumber = dataRode->header.pos("VAT_INV_NUMBER");
    int indTaxReportingScheme = dataRode->header.pos("TAX_REPORTING_SCHEME");
    int indTaxCountry = dataRode->header.pos("VAT_CALCULATION_IMPUTATION_COUNTRY");
    int indTaxCountryDeclaration = dataRode->header.pos("TAXABLE_JURISDICTION");
    int indTaxCollectionResponsibliity = -1;
    if (dataRode->header.contains("TAX_COLLECTION_RESPONSIBILITY")) {
        indTaxCollectionResponsibliity = dataRode->header.pos("TAX_COLLECTION_RESPONSIBILITY");
    }
    struct ShipmentTotal{
        double totalTaxed = 0.;
        double totalTaxes = 0.;
        QString countryVatAmazon;
        QString countrySaleDeclarationAmazon;
        QString regimeVatAmazon;
        Shipment *shipmentOrRefund;
    };
    QHash<QString, ShipmentTotal> id_shipmentOrRefunds;
    //int indItemName = dataRode->header.pos("ITEM_DESCRIPTION");
    for (auto elements : dataRode->lines) {
        QString transactionType = elements[indTransactionType];
        QString invoiceNumber = elements[indVatInvoiceNumber];
        QString vatScheme = elements[indTaxReportingScheme];
        QString saleChannel = elements[indSaleChannel];
        QString vatCollectResponsible;
        QString countryCodeFrom = elements[indFromCountry];
        QString countryCodeTo = elements[indDestCountry];
        if (invoiceNumber.contains("OSS") || vatScheme.contains("OSS")) {
            if (countryCodeTo == "GB") {
                countryCodeTo = "GB-NIR"; // TODO I should handle north ireland with country code GBIE and special vat rate
            } else if (countryCodeFrom == "GB") {
                countryCodeFrom = "GB-NIR"; // TODO I should handle north ireland with country code GBIE and special vat rate
            }
        }
        if (saleChannel != "MCF") {
            if (transactionType == "SALE") {
                QString orderId = elements[indAmazonOrderId];
                QString refundOrShipmentId = elements[indRefundOrShipmentId];
                if (elements[indRefundOrShipmentId] == "DVgYgfTG1") {
                    int TEMP=10;++TEMP;
                }
                QString vatNumber = elements[indBuyerVatNumber];
                if (orderId == "408-8663806-3875547") {
                    int TEMP=10;++TEMP;
                }
                QString currency = elements[indCurrency];
                QString subchannel = elements[indSubchannel];
                Address addressTo("", "", "", "",
                                  elements[indDestCity],
                                  elements[indDestPostcode],
                                  countryCodeTo,
                                  "", "");
                QHash<QString, QSharedPointer<ArticleSold>> articleSold;
                QString sku = elements[indSKU];
                //QString articleId = sku + "__" + refundOrShipmentId;
                Address addressFrom("", "", "", "",
                                    elements[indFromCity],
                                    elements[indFromPostcode],
                                    countryCodeFrom,
                                    "", "");
                double taxes = elements[indAmountTaxes].toDouble();
                QString dateString = elements[indDateShipping];
                QDate dateShipping = QDate::fromString(dateString, "dd-MM-yyyy");
                if (dateShipping.year() > maxYear) {
                    continue;
                }
                Q_ASSERT(dateShipping.isValid());
                orderMapping->minDate = qMin(QDateTime(dateShipping), orderMapping->minDate);
                orderMapping->maxDate = qMax(QDateTime(dateShipping), orderMapping->maxDate);
                QDateTime dateTimeShipping(dateShipping, QTime(23,59));
                QSharedPointer<Shipment> shipment(new Shipment(
                                                      refundOrShipmentId,
                                                      articleSold,
                                                      Shipping(0., 0., currency),
                                                      dateTimeShipping,
                                                      addressFrom,
                                                      currency,
                                                      taxes));
                if (indTaxCollectionResponsibliity >= 0) {
                    vatCollectResponsible = elements[indTaxCollectionResponsibliity];
                    if (vatCollectResponsible == "MARKETPLACE") {
                        shipment->setVatCollectResponsible(Shipment::VAT_RESPONSIBLE_MARKETPLACE);
                    }
                }
                shipment->setInvoiceNameMarketplace(invoiceNumber);
                shipment->setVatScheme(vatScheme);
                shipment->setFromAmazonVatReports(true);
                QHash<QString, QSharedPointer<Shipment>> shipments;
                shipments[refundOrShipmentId] = shipment;
                QSharedPointer<Order> order(
                            new Order(
                                QDateTime(),
                                orderId,
                                currency,
                                name(),
                                subchannel,
                                addressTo,
                                Address(),
                                Shipping(0., 0., currency),
                                true,
                                shipments));
                bool isBusinessOrder = vatNumber.size() > 3;
                bool isVatNumberValid = vatNumber.size() > 3;
                QString countryFrom = countryCodeFrom;
                /*
                    if (countryFrom == "CZ" && qAbs(taxes) < 0.001) {
                        int TEMP=10;++TEMP;
                    }
                    if (orderId == "407-0432955-8852310") {
                        int TEMP=10;++TEMP;
                    }
                    //*/
                if (isBusinessOrder) {
                    if (elements[indDestCountry] == countryCodeFrom) {
                        if (QStringList({"BE", "IT", "ES", "FR", "NL"})
                                .contains(elements[indDestCountry])) {
                            if (elements[indDestCountry]
                                    != ManagerCompanyVatParams::instance()->countryCodeCompany()) { /// TODO company country from settings
                                if (qAbs(taxes) > 0.001) {
                                    isVatNumberValid = false; /// Countries that have implemented non-resident domestic reverse charge mechanism are Belgium, France, Italy, Netherlands and Spain
                                }
                            }
                        }
                    }
                    else if (elements[indDestCountry] != countryCodeFrom
                             && qAbs(taxes) > 0.001) {
                        isVatNumberValid = false; /// Means VAT number is wrong and vat was charged
                    }
                }
                order->setIsBusinessCustomer(isBusinessOrder);
                order->setIsVatNumberValid(isVatNumberValid);
                if (!orderMapping->orderById.contains(orderId)) {
                    orderMapping->orderById[orderId] = order;
                    int yearShipping = dateShipping.year();
                    if (!orderMapping->ordersQuantityByDate.contains(yearShipping)) {
                        orderMapping->ordersQuantityByDate[yearShipping]
                                =  QHash<QString, QMap<QDate, int>>();
                    }
                    if (!orderMapping->ordersQuantityByDate[yearShipping].contains(subchannel)) {
                        orderMapping->ordersQuantityByDate[yearShipping][subchannel]
                                = QMap<QDate, int>();
                    }
                    if (!orderMapping->ordersQuantityByDate[yearShipping][subchannel].contains(dateShipping)) {
                        orderMapping->ordersQuantityByDate[yearShipping][subchannel][dateShipping] = 1;
                    } else {
                        ++orderMapping->ordersQuantityByDate[yearShipping][subchannel][dateShipping];
                    }
                } else {
                    orderMapping->orderById[orderId]->merge(*order.data());
                }
                if (!id_shipmentOrRefunds.contains(refundOrShipmentId))
                {
                    id_shipmentOrRefunds[refundOrShipmentId] = ShipmentTotal{};
                }
                id_shipmentOrRefunds[refundOrShipmentId].totalTaxed
                        += elements[indAmountTotal].toDouble();
                id_shipmentOrRefunds[refundOrShipmentId].totalTaxes
                        += elements[indAmountTaxes].toDouble();
                id_shipmentOrRefunds[refundOrShipmentId].regimeVatAmazon
                        = elements[indTaxReportingScheme];
                id_shipmentOrRefunds[refundOrShipmentId].countryVatAmazon
                        = elements[indTaxCountry];
                id_shipmentOrRefunds[refundOrShipmentId].countrySaleDeclarationAmazon
                        = Shipment::amazonCountryDeclToCode(elements[indTaxCountryDeclaration]);
                id_shipmentOrRefunds[refundOrShipmentId].shipmentOrRefund
                        = orderMapping->orderById[orderId]->getShipment(refundOrShipmentId).data();
            } else if (transactionType == "REFUND") {
                QString sku = elements[indSKU];
                QString orderId = elements[indAmazonOrderId];
                QString quantity = elements[indQty];
                QString productName = elements[indItemName];
                QString refundId = elements[indRefundOrShipmentId];
                QString dateString = elements[indDateShipping];
                QDate date = QDate::fromString(dateString, "dd-MM-yyyy");
                Q_ASSERT(date.isValid());
                QDateTime dateTime(date, QTime(23,59));
                QString articleId = sku + "__" + refundId;
                QString currency = elements[indCurrency];
                auto article = QSharedPointer<ArticleSold>(
                            new ArticleSold(refundId,
                                            sku,
                                            productName,
                                            ManagerSaleTypes::SALE_PRODUCTS,
                                            quantity.toInt(),
                                            elements[indAmountTotal].toDouble(),
                                            elements[indAmountTaxes].toDouble(),
                                            currency));
                if (orderMapping->refundById.contains(refundId)) {
                    orderMapping->refundById[refundId]->addArticleShipped(articleId, article);
                } else {
                    QHash<QString, QSharedPointer<ArticleSold>> articlesSold;
                    articlesSold[articleId] = article;
                    QSharedPointer<Refund> refund(
                                new Refund(refundId,
                                           orderId,
                                           articlesSold,
                                           Shipping(0., 0., currency),
                                           dateTime,
                                           Address(), //TODO I could find address then update
                                           elements[indCurrency],
                                           elements[indAmountTaxes].toDouble()));
                    if (indTaxCollectionResponsibliity >= 0) {
                        vatCollectResponsible = elements[indTaxCollectionResponsibliity];
                    }
                    refund->setVatCollectResponsible(vatCollectResponsible);
                    refund->setInvoiceNameMarketplace(invoiceNumber);
                    refund->setChannel(name());
                    refund->setSubchannel(elements[indSubchannel]);
                    // TODO refund->addChargedFee(-1, "Commission", itSku.key(), amazonFees);
                    int year = date.year();
                    orderMapping->refundById[refundId] = refund;
                    orderMapping->refundByOrderId.insert(orderId, refund);
                    orderMapping->refundByDate[year].insert(dateTime, refund);
                }
                if (!id_shipmentOrRefunds.contains(refundId))
                {
                    id_shipmentOrRefunds[refundId] = ShipmentTotal{};
                }
                id_shipmentOrRefunds[refundId].totalTaxed
                        += elements[indAmountTotal].toDouble();
                id_shipmentOrRefunds[refundId].totalTaxes
                        += elements[indAmountTaxes].toDouble();
                id_shipmentOrRefunds[refundId].regimeVatAmazon
                        = elements[indTaxReportingScheme];
                id_shipmentOrRefunds[refundId].countryVatAmazon
                        = elements[indTaxCountry];
                id_shipmentOrRefunds[refundId].countrySaleDeclarationAmazon
                        = Shipment::amazonCountryDeclToCode(elements[indTaxCountryDeclaration]);
                id_shipmentOrRefunds[refundId].shipmentOrRefund
                        = orderMapping->refundById[refundId].data();

            } else if (transactionType == "FC_TRANSFER") {
                QString sku = elements[indSKU];
                QString quantity = elements[indQty];
                QString dateString = elements[indDateShipping];
                QDate dateShipping = QDate::fromString(dateString, "dd-MM-yyyy");
                Q_ASSERT(dateShipping.isValid());
                if (!orderMapping->inventoryDeported[dateShipping.year()].contains(dateShipping)) {
                    orderMapping->inventoryDeported[dateShipping.year()][dateShipping] = QHash<QString, QHash<QString, QMultiHash<QString, int>>>();
                }
                if (!orderMapping->inventoryDeported[dateShipping.year()][dateShipping].contains(countryCodeFrom)) {
                    orderMapping->inventoryDeported[dateShipping.year()][dateShipping][countryCodeFrom]
                            = QHash<QString, QMultiHash<QString, int>>();
                }
                if (!orderMapping->inventoryDeported[dateShipping.year()][dateShipping][countryCodeFrom].contains(countryCodeTo)) {
                    orderMapping->inventoryDeported[dateShipping.year()][dateShipping][countryCodeFrom][countryCodeTo]
                            = QMultiHash<QString, int>();
                }
                //int qty = quantity.toInt();
                //QString itemName = elements[indItemName];
                orderMapping->inventoryDeported[dateShipping.year()][dateShipping][countryCodeFrom][countryCodeTo].insert(
                            sku, quantity.toInt());
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
    for (const auto &shipmentOrRefund : id_shipmentOrRefunds)
    {
        shipmentOrRefund.shipmentOrRefund->setAmazonVatInformations(
                    shipmentOrRefund.countryVatAmazon
                    , shipmentOrRefund.countrySaleDeclarationAmazon
                    , shipmentOrRefund.regimeVatAmazon
                    , shipmentOrRefund.totalTaxed
                    , shipmentOrRefund.totalTaxes);
    }
    if (orderMapping->minDate > orderMapping->maxDate) {
        orderMapping->minDate = QDateTime();
        orderMapping->maxDate = QDateTime();
    }
    return orderMapping;
}
//----------------------------------------------------------
QSharedPointer<OrdersMapping> OrderImporterAmazonUE::_loadReportOrdersFbm(const QString &fileName, int maxYear) const
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
    QString errorColumnText = QObject::tr("Il faut ajouter les colonnes «Identificateurs d'impôt de l'acheteur» et «Nom de la société de l'acheteur» dans Commandes > Rapport sur les commandes > Ajouter ou supprimer des colonnes de rapport sur la commande");
    int indBuyerTaxNumber = dataRode->header.pos(
                "buyer-tax-registration-id",
                errorColumnText);
    int indBuyerCompanyName = dataRode->header.pos(
                "buyer-company-name",
                errorColumnText);

    //TODO handle 2 types of header…and check for csv
    // => Create class to manage efficiently different header versions
    for (auto elements : dataRode->lines) {
        QDateTime dateTimeOrder = dateTimeFromString(elements[indPurchaseDate]);
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
                        totalPriceTaxed,
                        totalTaxes,
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
                                 totalTaxes + shippingTotalTaxes));
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
        QString vatNumber = elements[indBuyerTaxNumber];
        order->setVatNumber(vatNumber);
        QString companyName = elements[indBuyerCompanyName];
        order->setCompanyName(companyName);

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
QSharedPointer<OrdersMapping> OrderImporterAmazonUE::_loadReportPayments(
        const QString &fileName, int maxYear) const
{
    QString absFileName = QFileInfo(fileName).fileName();
    Q_ASSERT(!absFileName.contains("_ca_")
             && !absFileName.contains("_com_"));
    /*
    if (absFileName == "payment_fr_2021_02_25__to__2021_03_11.txt") {
        int TEMP=10;++TEMP;
    }
    //*/
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
        if (elements.size() > 0) {
            QString transactionType = elements[indTransactionType];
            QString merchentOrderId = elements[indMerhantOrderId];
            QString orderId = elements[indAmazonOrderId];
            if (orderId == "402-4429555-7470714")
            {
                int TEMP=10;++TEMP;
            }
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
            //orderMapping->minDate = qMin(dateTimeShipping, orderMapping->minDate);
            //orderMapping->maxDate = qMax(dateTimeShipping, orderMapping->maxDate);
            QString sku = elements[indSku];
            //int quantity = elements[indQuantityPurchased].toInt();
            // order id          shippingDate     sku            amount-type    amount-desc    trans-id
            //QHash<QString, QHash<QDateTime, QHash<QString, QHash<QString, QHash<QString, QHash<QString, double>>>>>> refundInfos;
            if (transactionType.toLower() == "order") {
                if (subchannel.contains("mazon.")) {
                    QString shipmentId = elements[indShipmentId];
                    QHash<QString, QSharedPointer<ArticleSold>> articles;
                    Address addressUnknown;
                    QSharedPointer<Shipment> shipment(
                                new Shipment(elements[indShipmentId],
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
                if (!amountDescription.contains("Goodwill")) { /// This is an extra refund from amazon refunded later, but with a different refund id that lead to an additionnal refund with a wrong amount
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
                }
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
                QList<QList<QVariant>> infoFees;
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
                                amountTaxed += itAmountDesc.value();
                                if (itAmountDesc.key().contains("Tax")) {
                                    amountTaxes += itAmountDesc.value();
                                    vatForRoundCorrection += itAmountDesc.value();;
                                }
                            } else if (itAmountType.key().contains("ItemFees")) {
                                amazonFees += itAmountDesc.value();
                                QString feeId = settlementId + "-" + QString::number(numLine);
                                infoFees << QList<QVariant>({feeId, itAmountDesc.key(), itSku.key(), itAmountDesc.value()});
                            } else {
                                QString orderId = itOrder.key();
                                QString amountDesc = itAmountDesc.key();
                                QString amountType = itAmountType.key();
                                Q_ASSERT(false);
                            }
                        }
                    }
                    QString articleId = itSku.key() + "__" + itRefundId.key();
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
                    for (auto itFee = infoFees.begin();
                         itFee != infoFees.end(); ++itFee) {
                        refund->addChargedFee(itFee->value(0).toString()
                                              , itFee->value(1).toString()
                                              , itFee->value(2).toString()
                                              , itFee->value(3).toInt());
                    }
                    /*
                    if (itOrder.key() == "028-0282662-8432336") {
                        double totalRefund = refund->getTotalPriceTaxed();
                        double totalRefundTaxes = refund->getTotalPriceTaxes();
                        int TEMP=10;++TEMP;
                    }
                    //*/
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
QDateTime OrderImporterAmazonUE::dateTimeFromString(
        const QString &string,
        bool isDayLight)
{
    //static QLocale localUSA(QLocale::English, QLocale::UnitedStates);
    //QString stringReplaced = QString(string).replace("2022 02:", "2022 03:")
            //.replace("2021-03-28 02", "2021-03-28 03");
    //QDateTime dateTime = localUSA.toDateTime(string, "dd.MM.yyyy hh:mm:ss UTC");
    //*
    QDateTime dateTime
            = QDateTime::fromString(
                string,
                "dd.MM.yyyy hh:mm:ss UTC");
                //*/
    //if (string == "27.03.2022 02:52:20 UTC") { //TODO Temporary workaround until we can make it work everywhere
      //  int TEMP=10;++TEMP;
    //}
    if (dateTime.isDaylightTime()) {
        dateTime.setTimeSpec(Qt::UTC);
    }
    if(!dateTime.isValid()) {
        dateTime = QDateTime::fromString(
                    string,
                    "yyyy-MM-dd hh:mm:ss UTC");
        if (dateTime.isDaylightTime()) {
            dateTime.setTimeSpec(Qt::UTC);
        }
        if (!dateTime.isValid()) {
            if (string.contains("+") || string.contains("-")) {
                QString stringUpdated = string;
                if (string.contains("+")) {
                    stringUpdated = string.split("+")[0];
                } else if (string.contains("-")) {
                    auto elements = string.split("-");
                    elements.takeLast();
                    stringUpdated = elements.join("-");
                }
                dateTime = QDateTime::fromString(
                            stringUpdated,
                            "yyyy-MM-ddThh:mm:ss");
            }
        }
    }
    if (dateTime.isDaylightTime()) {
        dateTime.setTimeSpec(Qt::UTC);
    }
    if (!dateTime.isValid() && !isDayLight) {
        dateTime = dateTimeFromString(
                    QString(string).replace("02:", "03:"), true);
    }
    Q_ASSERT(dateTime.isValid());
    return dateTime;
}
//----------------------------------------------------------
