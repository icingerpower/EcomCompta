#include "../common/utils/CsvReader.h"

#include "model/orderimporters/ShippingAddressesManager.h"

#include "OrderImporterCustom.h"
#include "OrderImporterCustomManager.h"
#include "OrderImporterCustomException.h"
#include "model/bookkeeping/ManagerSaleTypes.h"

using PAR = OrderImporterCustomParams;

QString OrderImporterCustom::REPORT_TYPE = QObject::tr("Commandes");
//==========================================================
OrderImporterCustom::OrderImporterCustom(const QString &paramsId)
    : AbstractOrderImporter ()
{
    m_paramsId = paramsId;
}
//==========================================================
QString OrderImporterCustom::name() const
{
    return OrderImporterCustomManager::instance()
            ->customParamsName(m_paramsId);
    // TODO move reports folder if name is changed
}
//==========================================================
QString OrderImporterCustom::invoicePrefix() const
{
    return "WEB";
}
//==========================================================
QString OrderImporterCustom::uniqueId() const
{
    return "OrderImporterCustom-" + name();
}
//==========================================================
QList<ReportType> OrderImporterCustom::reportTypes() const
{
    QList<ReportType> types;
    ReportType type;
    type.shortName = OrderImporterCustom::REPORT_TYPE;
    type.helpText = QObject::tr("Rapport de commandes par défaut");
    type.extensions << "csv";
    types << type;
    return types;
}
//==========================================================
QSharedPointer<OrdersMapping> OrderImporterCustom::loadReport(
        const QString &reportTypeName,
        const QString &fileName,
        int maxYear) const
{
    QSharedPointer<OrdersMapping> orderMapping(new OrdersMapping);
    orderMapping->addYears(12);
    auto params = OrderImporterCustomManager::instance()->customParams(m_paramsId);
    QString sep = params->valueAnyCol(PAR::ID_CSV_SEP);
    QString guill = params->valueAnyCol(PAR::ID_CSV_DEL);
    CsvReader reader(fileName, sep, guill);
    reader.readAll();
    const DataFromCsv *dataRode = reader.dataRode();
    QString currency = params->valueAnyCol(PAR::ID_CURRENCY);
    int indCurrency = -1;
    if (params->hasDefaultValue(PAR::ID_CURRENCY)) {
        currency = params->valueDefault(PAR::ID_CURRENCY);
    } else if (params->hasCol(PAR::ID_CURRENCY)) {
        indCurrency = dataRode->header.pos(params->valueCol(PAR::ID_CURRENCY));
    } else {
        OrderImporterCustomException exception;
        exception.setError(QObject::tr("Dans les paramètres des colonnes, vous devez indiquer la devise."));
        exception.raise();
    }
    if (!params->hasCol(PAR::ID_ORDER)) {
        OrderImporterCustomException exception;
        exception.setError(QObject::tr("Dans les paramètres des colonnes, vous devez indiquer le numéro de commande unique."));
        exception.raise();
    }
    if (!params->hasCol(PAR::ID_NAME)
            || !params->hasCol(PAR::ID_STREET1)
            || !params->hasCol(PAR::ID_CITY)
            || !params->hasCol(PAR::ID_POSTAL_CODE)
            || !params->hasCol(PAR::ID_COUNTRY_CODE)) {
        OrderImporterCustomException exception;
        exception.setError(QObject::tr("Dans les paramètres des colonnes, vous devez indiquer les paramètres d'adresses (nom, rue, code postal, code pays et ville."));
        exception.raise();
    }
    if (!params->hasCol((PAR::ID_SKU))) {
        OrderImporterCustomException exception;
        exception.setError(QObject::tr("Dans les paramètres des colonnes, vous devez indiquer le code SKU de l'article vendu."));
        exception.raise();
    }
    if (!params->hasCol((PAR::ID_ARTICLE_NAME))) {
        OrderImporterCustomException exception;
        exception.setError(QObject::tr("Dans les paramètres des colonnes, vous devez indiquer le nom de l'article vendu."));
        exception.raise();
    }
    if (!params->hasCol((PAR::ID_QUANTITY))) {
        OrderImporterCustomException exception;
        exception.setError(QObject::tr("Dans les paramètres des colonnes, vous devez indiquer les quantités de l'article vendu."));
        exception.raise();
    }

    int indBuyerName = dataRode->header.pos(params->valueCol(PAR::ID_NAME));
    int indAddress1 = dataRode->header.pos(params->valueCol(PAR::ID_STREET1));
    int indCity = dataRode->header.pos(params->valueCol(PAR::ID_CITY));
    int indPostalCode = dataRode->header.pos(params->valueCol(PAR::ID_POSTAL_CODE));
    int indCountryCode = dataRode->header.pos(params->valueCol(PAR::ID_COUNTRY_CODE));

    int indSku = dataRode->header.pos(params->valueCol(PAR::ID_SKU));
    int indProductName = dataRode->header.pos(params->valueCol(PAR::ID_ARTICLE_NAME));
    int indQuantityShipped = dataRode->header.pos(params->valueCol(PAR::ID_QUANTITY));
    QHash<QString, double> orderTotalById;
    QHash<QString, QHash<QDateTime, double>> refundInfos;

    for (auto elements : dataRode->lines) {
        QString address2;
        if (params->hasCol(PAR::ID_STREET2)) {
            int indAddress2 = dataRode->header.pos(params->valueCol(PAR::ID_STREET2));
            address2 = elements[indAddress2];
        }
        if (indCurrency != -1) {
            currency = elements[indCurrency];
        }
        QString countryCodeTo = elements[indCountryCode];
        QString state;
        if (params->hasCol(PAR::ID_STATE)) {
            int indState = dataRode->header.pos(params->valueCol(PAR::ID_STATE));
            state = elements[indState];
        }
        Address addressTo(elements[indBuyerName],
                          elements[indAddress1],
                          address2,
                          "",
                          elements[indCity],
                          elements[indPostalCode],
                          countryCodeTo,
                          state,
                          "");
        Address addressBillingTo(addressTo); //TODO
        double shippingArticleTotal = 0;
        if (params->hasCol((PAR::ID_ARTICLE_SHIPPING))) {
            int indArticleShipping = dataRode->header.pos(params->valueCol(PAR::ID_ARTICLE_SHIPPING));
            shippingArticleTotal = elements[indArticleShipping].toDouble();
        }
        Shipping shippingArticle(
                    shippingArticleTotal,
                    0.,
                    currency);
        Shipping shippingExtra(0., 0., currency);
        QString orderId;
        QString colInfo = params->valueCol(PAR::ID_ORDER);
        if (colInfo.contains(",")) {
            QStringList colElements = colInfo.split(",");
            for (auto element : colElements){
                if (element.startsWith("\"")) { //TODO create a function for that kind of column with comma that will also check if number with toDouble
                    element.replace("\"", "");
                    orderId += element;
                } else {
                    int indOrderId = dataRode->header.pos(element);
                    orderId += elements[indOrderId];
                }
            }
        } else {
            int indOrderId = dataRode->header.pos(params->valueCol(PAR::ID_ORDER));
            orderId = elements[indOrderId];
        }
        if (params->hasCol(PAR::ID_TOTAL_PAID_ORDER)){
            int indTotalOrder = dataRode->header.pos(params->valueCol(PAR::ID_TOTAL_PAID_ORDER));
            orderTotalById[orderId] = elements[indTotalOrder].toDouble();
        }
        QString shipmentId = orderId;
        QString sku = elements[indSku];
        QString articleId = orderId + "-" + sku;
        int quantityShipped = elements[indQuantityShipped].toInt();
        double totalArticlePrice = 0.;
        if (params->hasCol(PAR::ID_ARTICLE_UNIT_PRICE)) {
            int indUnitPrice = dataRode->header.pos(params->valueCol(PAR::ID_ARTICLE_UNIT_PRICE));
            double articleUnitPrice = elements[indUnitPrice].toDouble();
            totalArticlePrice = articleUnitPrice * quantityShipped;
        } else if (params->hasCol(PAR::ID_ARTICLE_SUM_PRICE)) {
            // TODO
        } else {
            OrderImporterCustomException exception;
            exception.setError(QObject::tr("Dans les paramètres des colonnes, vous devez indiquer les colonnes pour connaître le prix unitaire."));
            exception.raise();
        }
        if (params->hasCol(PAR::ID_ARTICLE_DISCOUNT)) {
            int indDiscount = dataRode->header.pos(params->valueCol(PAR::ID_ARTICLE_DISCOUNT));
            double discount = elements[indDiscount].toDouble();
            totalArticlePrice -= qAbs(discount);
        }
            /*
        } else if (params->hasCol(PAR::ID_ARTICLE_DISCOUNT_TO_DEDUCT)) {
            int indDiscountToDeduct = dataRode->header.pos(params->valueCol(PAR::ID_ARTICLE_DISCOUNT_TO_DEDUCT));
            double discount = elements[indDiscountToDeduct].toDouble();
            totalArticlePrice -= discount;
            //*/
        QSharedPointer<ArticleSold> article(
                    new ArticleSold(
                        articleId,
                        sku,
                        elements[indProductName],
                        ManagerSaleTypes::SALE_PRODUCTS,
                        quantityShipped,
                        totalArticlePrice,
                        0.,
                        currency));
        article->setShipping(shippingArticle);
        QHash<QString, QSharedPointer<ArticleSold>> articles;
        articles[articleId] = article;
        QString shippingAddressId = getDefaultShippingAddressId();
        Address addressFrom //TODO handle exception if no default shipping address
                = ShippingAddressesManager::instance()->getAddress(
                    shippingAddressId) ;
        if (addressFrom.isEmpty()) {
            OrderImporterCustomException exception;
            exception.setError(QObject::tr("Dans les paramètres, vous devez choisir une addresse d'expédition par défaut."));
            exception.raise();
        }

        QDateTime dateTime;
        if (params->hasCol(PAR::ID_DATE) && (params->hasCol(PAR::ID_DATE_FORMAT)
                                             || params->hasDefaultValue(PAR::ID_DATE_FORMAT))) {
            int indDate = dataRode->header.pos(params->valueCol(PAR::ID_DATE));
            QString dateFormat = params->valueAnyCol(PAR::ID_DATE_FORMAT);
            QDate date = QDate::fromString(elements[indDate], dateFormat);
            QTime time(0, 0);
            if (params->hasCol(PAR::ID_TIME) && (params->hasCol(PAR::ID_TIME_FORMAT)
                                                 || params->hasDefaultValue(PAR::ID_TIME_FORMAT))) {
                int indTime = dataRode->header.pos(params->valueCol(PAR::ID_TIME));
                QString timeFormat = params->valueAnyCol(PAR::ID_TIME_FORMAT);
                time = QTime::fromString(elements[indTime], timeFormat);
                Q_ASSERT(time.isValid());
            }
            dateTime = QDateTime(date, time);
            //TODO get time if available
        } else if (params->hasCol(PAR::ID_DATE_TIME) && (params->hasCol(PAR::ID_DATE_TIME_FORMAT)
                                             || params->hasDefaultValue(PAR::ID_DATE_TIME))) {
            int indDateTime = dataRode->header.pos(params->valueCol(PAR::ID_DATE_TIME));
            QString dateTimeFormat = params->valueAnyCol(PAR::ID_DATE_TIME_FORMAT);
            dateTime = QDateTime::fromString(elements[indDateTime], dateTimeFormat);
        } else {
            OrderImporterCustomException exception;
            exception.setError(QObject::tr("Dans les paramètres des colonnes, vous devez indiquer les colonnes pour connaître au moins la date."));
            exception.raise();
        }
        Q_ASSERT(!dateTime.date().isNull());
        if (dateTime.date().year() > maxYear) {
            continue;
        }
        orderMapping->maxDate = qMax(dateTime, orderMapping->maxDate);
        orderMapping->minDate = qMin(dateTime, orderMapping->minDate);
        QSharedPointer<Shipment> shipment(
                    new Shipment(shipmentId,
                                 articles,
                                 Shipping(0., 0., currency), // TODO add shipping?
                                 dateTime,
                                 addressFrom,
                                 currency,
                                 0.));
        QHash<QString, QSharedPointer<Shipment>> shipments;
        shipments[shipment->getId()] = shipment;
        QString subChannel = name();
        if (params->hasCol(PAR::ID_SUBCHANNEL)) {
            int indSubChannel = dataRode->header.pos(params->valueCol(PAR::ID_SUBCHANNEL));
            subChannel = elements[indSubChannel];
        } else if (params->hasDefaultValue(PAR::ID_SUBCHANNEL)) {
            subChannel = params->valueDefault(PAR::ID_SUBCHANNEL);
        } else {
            OrderImporterCustomException exception;
            exception.setError(QObject::tr("Dans les paramètres des colonnes, vous devez indiquer la colonne canal de vente (par exemple le nom du site)."));
            exception.raise();
        }
        QSharedPointer<Order> order(
                    new Order(
                        dateTime,
                        orderId,
                        currency,
                        name(),
                        subChannel,
                        addressTo,
                        addressBillingTo,
                        shippingExtra,
                        false,
                        shipments));
        order->setShippedBySeller(true);
        bool isBusinessCustomer = false;
        QString vatNumber;
        if (params->hasCol(PAR::ID_BUSINESS_VAT_NUMBER)) {
            int indVatNumber = dataRode->header.pos(
                        params->valueCol(PAR::ID_BUSINESS_VAT_NUMBER));
            vatNumber = elements[indVatNumber]; // TODO check vat number valid
            isBusinessCustomer = true;
        }
        order->setIsBusinessCustomer(isBusinessCustomer);
        if (isVatToRecompute()) {
            order->setVatToRecompute(true);
        }
        if (params->hasCol(PAR::ID_VALUES_REFUND_TO_DEDUCT)
                && !params->valueDefault(PAR::ID_VALUES_REFUND_TO_DEDUCT).isEmpty()) {
            int indRefundValues = dataRode->header.pos(
                        params->valueCol(PAR::ID_VALUES_REFUND_TO_DEDUCT));
            QStringList validValues = params->valueDefault(PAR::ID_VALUES_REFUND_TO_DEDUCT).split(",");
            QString colValue = elements[indRefundValues];
            for (auto validValue : validValues) {
                if (colValue.contains(validValue)) {
                    if (params->hasCol(PAR::ID_DATE_REFUNDED)) {
                        // TODO
                    } else {
                        continue; /// We skip because no date
                    }
                }
            }
        }
        if (shipmentId.contains("1325")) {
            int TEMP=10;++TEMP;
        }
        shipment->computeVatForRoundCorrection();
        /// Save order or merge it
        int yearShipment = dateTime.date().year(); /// Amazon use shippig date for invoicne / vat report
        if (orderMapping->orderById.contains(orderId)) {
            orderMapping->orderById[orderId]->merge(*order.data());
        } else {
            orderMapping->orderById[orderId] = order;
            int yearOrder = dateTime.date().year();
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
            orderMapping->shipmentById[shipmentId]->merge(*shipment.data()); // should not be needed as done by order->merge
        } else {
            orderMapping->shipmentById[shipmentId] = shipment;
            orderMapping->shipmentByDate[yearShipment].insert(dateTime, shipment);
        }
        /// Save refund information
        if (params->hasCol(PAR::ID_TOTAL_REFUNDED)
                && (params->hasCol(PAR::ID_DATE_REFUNDED)
                    || params->hasCol(PAR::ID_DATE_TIME_REFUNDED))) {
            QDateTime refundDateTime;
            if (params->hasCol(PAR::ID_DATE_REFUNDED) && (params->hasCol(PAR::ID_DATE_FORMAT)
                                                          || params->hasDefaultValue(PAR::ID_DATE_FORMAT))) {
                int indDate = dataRode->header.pos(params->valueCol(PAR::ID_DATE_REFUNDED));
                QString dateFormat = params->valueAnyCol(PAR::ID_DATE_FORMAT);
                QDate date = QDate::fromString(elements[indDate], dateFormat);
                refundDateTime = QDateTime(date, QTime(0,0));
            } else if (params->hasCol(PAR::ID_DATE_TIME_REFUNDED) && (params->hasCol(PAR::ID_DATE_TIME_FORMAT)
                                                             || params->hasDefaultValue(PAR::ID_DATE_TIME))) {
                int indDateTime = dataRode->header.pos(params->valueCol(PAR::ID_DATE_TIME_REFUNDED));
                QString dateTimeFormat = params->valueDefault(PAR::ID_DATE_TIME_FORMAT);
                refundDateTime = QDateTime::fromString(elements[indDateTime], dateTimeFormat);
            }
            Q_ASSERT(refundDateTime.isValid());
            QDateTime orderDateTime;
            if (orderDateTime.daysTo(refundDateTime) < 1) {
                //if (orderDateTime.date().month() < 12 || orderDateTime.date().day() < 31) {
                refundDateTime = order->getDateTime().addDays(1); /// Workaround to avoid having vat of refund trying to be computed before order
                //}
            }
            int indTotalRefunded = dataRode->header.pos(
                        params->valueCol(PAR::ID_TOTAL_REFUNDED));
            double refund = elements[indTotalRefunded].toDouble();
            if (qAbs(refund) > 0.001) {
                if (!refundInfos.contains((orderId))) {
                    refundInfos[orderId] = QHash<QDateTime, double>();
                }
                refundInfos[orderId][refundDateTime] = refund;
            }
        }
    }
    if (params->hasCol(PAR::ID_TOTAL_PAID_ORDER)){
        for (auto order : orderMapping->orderById) {
            double currentTotal = order->getTotalPriceTaxed();
            double totalPaid = orderTotalById[order->getId()];
            double diff = totalPaid - currentTotal;
            if (qAbs(diff) > 0.005) {
                /*
                auto shipments = order->getShipments();
                QString diffTitle = QObject::tr("Promo");
                if (diff < 0) {
                    diffTitle = QObject::tr("Frais de traitement et livraison");
                }
                //*/
                auto shipment = order->getShipmentFirst();
                Shipping shipping = shipment->getShipping();
                shipping.setTotalPriceTaxed(shipping.totalPriceTaxed() + diff);
                shipment->setShipping(shipping);
            }
        }
    }
    for (auto it = refundInfos.begin();
         it != refundInfos.end(); ++it) {
        for (auto it2 = it.value().begin();
             it2 != it.value().end(); ++it2) {
            QString orderId = it.key();
            auto order = orderMapping->orderById[orderId];
            double totalOrder = order->getTotalPriceTaxed();
            QDateTime dateTime = it2.key();
            double refundTotal = it2.value();
            double ratio = qAbs(refundTotal/totalOrder);
            auto shipment = order->getShipmentFirst();
            QHash<QString, QSharedPointer<ArticleSold>> articlesSold;
            for (auto articleSold : shipment->getArticlesShipped()) {
                QSharedPointer<ArticleSold> articleSoldCopy(
                            new ArticleSold(
                                articleSold->getShipmentItemId(),
                                articleSold->getSku(),
                                articleSold->getName(),
                                ManagerSaleTypes::SALE_PRODUCTS,
                                articleSold->getUnits(),
                                -articleSold->getTotalPriceTaxed() * ratio,
                                0.,
                                articleSold->getCurrency()));
                auto articleShipping = articleSold->getShipping();
                articleShipping.setTotalPriceTaxed(articleShipping.totalPriceTaxed() * -ratio);
                articleSoldCopy->setShipping(articleShipping);
                articlesSold[articleSoldCopy->getShipmentItemId()] = articleSoldCopy;
            }
            QString refundId = orderId + "-refund-1";
            Shipping shipping = shipment->getShipping();
            double shippingDiff1 = shipment->getShipping().totalPriceTaxed();
            double shippingDiff2 = order->getShipping().totalPriceTaxed();
            double shippingDiff = (-shippingDiff1 - shippingDiff2) * ratio;
            shipping.setTotalPriceTaxed(shippingDiff);
            QSharedPointer<Refund> refund(
                        new Refund(
                            refundId,
                            orderId,
                            articlesSold,
                            shipping,
                            dateTime,
                            shipment->getAddressFrom(),
                            shipment->getCurrency(),
                            0.));
            refund->setChannel(name());
            refund->setSubchannel(shipment->subchannel());
            refund->init(order.data());
            orderMapping->refundById[refundId] = refund;
            orderMapping->refundByOrderId.insert(orderId, refund);
            int year = dateTime.date().year();
            if (!orderMapping->refundByDate.contains(year)) {
                orderMapping->refundByDate[year] = QMultiMap<QDateTime, QSharedPointer<Refund>>();
            }
            orderMapping->refundByDate[year].insert(dateTime, refund);
        }
    }
    for (auto order : orderMapping->orderById) {
        for (auto shipment : order->getShipments()) {
            shipment->addReportFrom(orderMapping->maxDate.date(), reportTypeName);
        }
    }
    for (auto refund : orderMapping->refundById) {
        refund->addReportFrom(orderMapping->maxDate.date(), reportTypeName);
    }
    orderMapping->removeEmptyYears();
    if (orderMapping->minDate > orderMapping->maxDate) {
        orderMapping->minDate = QDateTime();
        orderMapping->maxDate = QDateTime();
    }
    return orderMapping;
}
//==========================================================
QList<QStringList> OrderImporterCustom::reportForOrderComplete(
        const Order *) const
{
    static QList<QStringList> reports = {{OrderImporterCustom::REPORT_TYPE}};
    return reports;
}
//==========================================================
