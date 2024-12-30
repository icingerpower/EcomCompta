#include "Order.h"
#include "AbstractOrderImporter.h"
#include "../common/currencies/CurrencyRateManager.h"
#include "model/SettingManager.h"
#include "model/CustomerManager.h"



//==========================================================
Order::Order(const QDateTime &dateTime,
             const QString &id,
             const QString &currency,
             const QString &channel,
             const QString &subchannel,
             const Address &addressTo,
             const Address &addressBillingTo,
             const Shipping &shippingExtra,
             bool isFromMarketplace,
             const QHash<QString, QSharedPointer<Shipment> > &shipments)
{
    m_id = id;
    Q_ASSERT(!m_id.endsWith("\""));
    m_currency = currency;
    m_shippedBySeller = true;
    m_channel = channel;
    m_subchannel = subchannel;
    m_dateTime = dateTime;
    m_addressTo = addressTo;
    m_addressBillingTo = addressBillingTo;
    m_shippingExtra = shippingExtra;
    m_isBusinessCustomer = false;
    m_isVatNumberValid = false;
    m_shippingExtra.init(this);
    m_isFromMarketplace = isFromMarketplace;
    setShipments(shipments);
}
//==========================================================
void Order::merge(const Order &order)
{
    if (m_id == "22081721322R0P6") {
        int TEMP=10;++TEMP;
    }
    if (m_dateTime.isNull()) {
        m_dateTime = order.m_dateTime;
    }
    if (m_channel == "") {
        m_channel = order.m_channel;
    }
    if (m_subchannel == "") {
        m_channel = order.m_subchannel;
    }
    if (!m_shippedBySeller || !order.m_shippedBySeller) {
        m_shippedBySeller = false;
    }
    //m_reportsFrom.unite(order.reportsFrom());
    m_addressTo.merge(order.m_addressTo);
    m_addressBillingTo.merge(order.m_addressBillingTo);
    QSet<QString> shipmentIdsNoArticles;
    QSet<QString> shipmentIdsArticles;
    for (auto it=order.m_shipments.begin();
         it != order.m_shipments.end();
         ++it) {
        if (m_shipments.contains(it.key())) {
            m_shipments[it.key()]->merge(*it.value().data());
        } else {
            m_shipments[it.key()] = it.value();
            m_shipments[it.key()]->init(this);
        }
        if (m_shipments[it.key()]->getArticleCount() == 0) {
            shipmentIdsNoArticles << it.key();
        } else {
            shipmentIdsArticles << it.key();
        }
    }
    if (shipmentIdsNoArticles.size() > 0) {
        if (shipmentIdsArticles.size() > 0) {
            //Q_ASSERT(shipmentIdsArticles.size() == 1); /// If > 1, needs to implement on which shipment will will merge the shipments without articles. This can happen in FBM order with 2 articles shipped on different time
            // Can also happens when same article ID was shipped in several times in FBA order, CF 404-0760153-3734736
            for (auto id : shipmentIdsNoArticles) {
                auto shipment = m_shipments.take(id);
                QString idToUpdate = shipmentIdsArticles.values()[0];
                m_shipments[idToUpdate]->setPaymentId(
                            shipment->getPaymentId());
            }
        }
    }
    if (m_shippingExtra.isNull()) {
        m_shippingExtra = order.m_shippingExtra;
        m_shippingExtra.init(this);
    }
    m_vatToRecompute |= order.m_vatToRecompute;
    m_isBusinessCustomer |= order.m_isBusinessCustomer;
    m_isVatNumberValid |= order.m_isVatNumberValid;
}
//==========================================================
QMultiHash<QString, QSharedPointer<ArticleSold> > Order::articlesSold() const
{
    QMultiHash<QString, QSharedPointer<ArticleSold>> articles;
    for (auto shipment : m_shipments) {
        auto articlesShipment = shipment->articlesShipped();
        for (auto itArt = articlesShipment.begin();
             itArt != articlesShipment.end(); ++itArt) {
            articles.insert(itArt.key(), itArt.value());
        }
    }
    return articles;
}
//==========================================================
QHash<QString, QSharedPointer<ArticleSold> > Order::articlesSoldUnited() const
{
    QHash<QString, QSharedPointer<ArticleSold>> articles;
    for (auto shipment : m_shipments) {
        auto articlesShipment = shipment->articlesShipped();
        for (auto itArt = articlesShipment.begin();
             itArt != articlesShipment.end(); ++itArt) {
            QString artId = itArt.key();
            if (articles.contains(artId)) {
                auto articleNew = itArt.value();
                int unitsTotal = articleNew->getUnits() + articles[artId]->getUnits();
                int totalPriceTaxed = articleNew->getTotalPriceTaxed() + articles[artId]->getTotalPriceTaxed();
                int totalTaxes = articleNew->getTotalPriceTaxes() + articles[artId]->getTotalPriceTaxes();
                QSharedPointer<ArticleSold> articleMerged(
                            new ArticleSold(
                                artId,
                                articleNew->getSku(),
                                articleNew->getName(),
                                articleNew->getSaleType(),
                                unitsTotal,
                                totalPriceTaxed,
                                totalTaxes,
                                articleNew->getCurrency()
                                ));
                articleMerged->init(shipment.data());
                // TODO shall I init from a shipment?
                articles[artId] = articleMerged;
            } else {
                articles.insert(artId, itArt.value());
            }
        }
    }
    return articles;
}
//==========================================================
double Order::getVatForRoundCorrection() const
{
    double vat = 0.;
    for (auto shipment : m_shipments) {
        vat += shipment->getVatForRoundCorrection();
    }
    return vat;
}
//==========================================================
double Order::getTotalPriceTaxed() const
{
    double price = m_shippingExtra.totalPriceTaxed();
    for (auto shipment : m_shipments) {
        price += shipment->getTotalPriceTaxed();
    }
    return price;
}
//==========================================================
double Order::getTotalPriceTaxes() const
{
    double price = m_shippingExtra.totalTaxes();
    for (auto shipment : m_shipments) {
        price += shipment->getTotalPriceTaxes();
    }
    return price;
}
//==========================================================
double Order::getTotalPriceUntaxed() const
{
    double value = getTotalPriceTaxed() - getTotalPriceTaxes();
    return value;
}
//==========================================================
QMap<QString, double> Order::getChargedFees() const
{
    QMap<QString, double> fees;
    for (auto shipment : m_shipments) {
        auto shipmentFees = shipment->getChargedFeesBySKU();
        for (auto itSku = shipmentFees.begin();
             itSku != shipmentFees.end();
             ++itSku) {
            for (auto itFee = itSku.value().begin();
                 itFee != itSku.value().end();
                 ++itFee) {
                if (fees.contains(itFee.key())) {
                    fees[itFee.key()] += itFee.value().amountTotal;
                } else {
                    fees[itFee.key()] = itFee.value().amountTotal;
                }
            }
        }
    }
    return fees;
}
//==========================================================
double Order::getTotalPriceTaxedConverted() const
{
    double converted =
            CurrencyRateManager::instance()->convert(
                getTotalPriceTaxed(),
                getCurrency(),
                CustomerManager::instance()->getSelectedCustomerCurrency(),
                getDateTime().date());
    return converted;
}
//==========================================================
double Order::getTotalPriceTaxesConverted() const
{
    double converted =
            CurrencyRateManager::instance()->convert(
                getTotalPriceTaxes(),
                getCurrency(),
                CustomerManager::instance()->getSelectedCustomerCurrency(),
                getDateTime().date());
    return converted;
}
//==========================================================
double Order::getTotalPriceUntaxedConverted() const
{
    double converted =
            CurrencyRateManager::instance()->convert(
                getTotalPriceUntaxed(),
                getCurrency(),
                CustomerManager::instance()->getSelectedCustomerCurrency(),
                getDateTime().date());
    return converted;
}
//==========================================================
int Order::getArticleUnitCount() const
{
    int count = 0;
    for (auto shipment : m_shipments) {
        count += shipment->getUnitCounts();
    }
    return count;
}
//==========================================================
/*
void Order::addReportFrom(const QString &reportName)
{
    m_reportsFrom << reportName;
}
//*/
//==========================================================
QSet<QString> Order::reportsFrom() const
{
    auto shipments = getShipments().values();
    auto reportsFrom = shipments[0]->getReportsFrom();
    for (int i = 1; i < getShipmentCount(); ++i) {
        auto otherReportsFrom = shipments[i]->getReportsFrom();
        for (auto itOtherReportFrom = otherReportsFrom.begin();
             itOtherReportFrom != otherReportsFrom.end(); ++itOtherReportFrom) {
            reportsFrom[itOtherReportFrom.key()] = itOtherReportFrom.value();
        }
    }
    QSet<QString> reportsFromSet;
    for (auto itReport = reportsFrom.begin();
         itReport != reportsFrom.end(); ++itReport) {
        reportsFromSet << itReport.key();
    }
    return reportsFromSet;
}
//==========================================================
QList<QStringList> Order::reportCombinationMissing() const
{
    QMultiHash<QString, QStringList> reportForOrderCompleteMap;
    for (auto importer : AbstractOrderImporter::allImporters()) {
        auto importerReportForOrderCompleteMap
                = importer->reportForOrderCompleteMap(this);
        reportForOrderCompleteMap.unite(importerReportForOrderCompleteMap);
    }
    QList<QStringList> possibleCombination;
    for (auto it = reportForOrderCompleteMap.begin();
         it != reportForOrderCompleteMap.end();
         ++it) {
        auto reportsFrom = this->reportsFrom();
        if (reportsFrom.contains(it.key())) {
            if (it.value().toSet() == reportsFrom) {
                return QList<QStringList>();
            } else {
                possibleCombination << it.value();
            }
        }
    }
    return possibleCombination;
}
//==========================================================
QDateTime Order::getDateTime() const
{
    return m_dateTime;
}
//==========================================================
void Order::setDateTime(const QDateTime &dateTime)
{
    m_dateTime = dateTime;
}
//==========================================================
QString Order::getId() const
{
    return m_id;
}
//==========================================================
void Order::setId(const QString &id)
{
    m_id = id;
}
//==========================================================
QSet<int> Order::shipmentYears() const
{
    QSet<int> years;
    for (auto shipment : m_shipments) {
        years << shipment->getDateTime().date().year();
    }
    return years;
}
//==========================================================
QString Order::getChannel() const
{
    return m_channel;
}
//==========================================================
void Order::setChannel(const QString &channel)
{
    m_channel = channel;
}
//==========================================================
Shipping Order::getShipping() const
{
    return m_shippingExtra;
}
//==========================================================
void Order::setShipping(const Shipping &shipping)
{
    m_shippingExtra = shipping;
}
//==========================================================
Address Order::getAddressTo() const
{
    return m_addressTo;
}
//==========================================================
void Order::setAddressTo(const Address &addressTo)
{
    m_addressTo = addressTo;
}
//==========================================================
QString Order::getSubchannel() const
{
    return m_subchannel;
}
//==========================================================
void Order::setSubchannel(const QString &subchannel)
{
    m_subchannel = subchannel;
}
//==========================================================
Address Order::getAddressBillingTo() const
{
    return m_addressBillingTo;
}
//==========================================================
void Order::setAddressBillingTo(const Address &addressBillingTo)
{
    m_addressBillingTo = addressBillingTo;
}
//==========================================================
bool Order::getVatToRecompute() const
{
    return m_vatToRecompute;
}
//==========================================================
void Order::setVatToRecompute(bool vatToRecompute)
{
    m_vatToRecompute = vatToRecompute;
}
//==========================================================
bool Order::isBusinessCustomer() const
{
    return m_isBusinessCustomer;
}
//==========================================================
void Order::setIsBusinessCustomer(bool isBusinessCustomer)
{
    if (m_id == "403-7789164-3791527") {
        int TEMP=10;++TEMP;
    }
    m_isBusinessCustomer = isBusinessCustomer;
}
//==========================================================
QString Order::getVatNumber() const
{
    return m_vatNumber;
}
//==========================================================
void Order::setVatNumber(const QString &vatNumber)
{
    m_vatNumber = vatNumber;
}
//==========================================================
QString Order::getCompanyName() const
{
    return m_companyName;
}
//==========================================================
void Order::setCompanyName(const QString &companyName)
{
    m_companyName = companyName;
}
//==========================================================
bool Order::getShippedBySeller() const
{
    return m_shippedBySeller;
}
//==========================================================
void Order::setShippedBySeller(bool shippedBySeller)
{
    m_shippedBySeller = shippedBySeller;
}
//==========================================================
QString Order::getCurrency() const
{
    return m_currency;
}
//==========================================================
void Order::setCurrency(const QString &currency)
{
    m_currency = currency;
}
//==========================================================
bool Order::isVatNumberValid() const
{
    return m_isVatNumberValid;
}
//==========================================================
void Order::setIsVatNumberValid(bool isVatNumberValid)
{
    if (m_id == "407-0432955-8852310") {
        int TEMP=10;++TEMP;
    }
    m_isVatNumberValid = isVatNumberValid;
}
//==========================================================
bool Order::isFromMarketplace() const
{
    return m_isFromMarketplace;
}
//==========================================================
void Order::setIsFromMarketplace(bool newIsFromMarketplace)
{
    m_isFromMarketplace = newIsFromMarketplace;
}
//==========================================================
void Order::addShipment(QSharedPointer<Shipment> shipment)
{
    m_shipments[shipment->getId()] = shipment;
    shipment->init(this);
}
//==========================================================
QHash<QString, QSharedPointer<Shipment> > Order::getShipments() const
{
    return m_shipments;
}
//==========================================================
QSharedPointer<Shipment> Order::getShipmentFirst() const
{
    return m_shipments.begin().value();
}
//==========================================================
void Order::setShipments(
        const QHash<QString, QSharedPointer<Shipment> > &shipments)
{
    m_shipments = shipments;
    for (auto shipment : m_shipments) {
        shipment->init(this);
    }
}
//==========================================================
int Order::getShipmentCount() const
{
    return m_shipments.size();
}
//==========================================================
bool Order::isShippedNextYear() const
{
    int yearOrder = m_dateTime.date().year();
    for (auto shipment : m_shipments) {
        if (shipment->getDateTime().date().year() > yearOrder) {
            return true;
        }
    }
    return false;
}
//==========================================================
bool Order::containsCountry(const QString &countryCode) const
{
    bool contains = m_addressTo.countryCode() == countryCode;
    if (!contains) {
        for (auto shipment : m_shipments) {
            if (shipment->getAddressFrom().countryCode() == countryCode) {
                return true;
            }
        }
    }
    return contains;
}
//==========================================================
bool Order::containsSku(const QString &sku) const
{
    for (auto shipment : m_shipments) {
        if (shipment->containsSku(sku)) {
            return true;
        }
    }
    return false;
}
//==========================================================
QStringList Order::paymentIds() const
{
    QStringList ids;
    for (auto it = m_shipments.begin();
         it != m_shipments.end();
         ++it) {
        ids << it.value()->getPaymentId();
    }
    return ids;
}
//==========================================================
