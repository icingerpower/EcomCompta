#include <QtCore/qstringlist.h>

#include "model/SettingManager.h"

#include "Refund.h"
#include "Order.h"


//==========================================================
Refund::Refund(const QString &id,
               const QString &orderId,
               const QHash<QString, QSharedPointer<ArticleSold>> &articlesShipped,
               const Shipping &shipping,
               const QDateTime &dateTime,
               const Address &addressTo,
               const QString &currency,
               double vatForRoundCorrection)
    : Shipment(id,
               articlesShipped,
               shipping,
               dateTime,
               Address(),
               currency,
               vatForRoundCorrection)
{
    m_orderId = orderId;
    m_addressTo = addressTo;
    m_retrieveAllShipmentsOnInit = false;
    m_wasGuessed = false;
}
//==========================================================
void Refund::init(Order *order)
{
    Shipment::init(order);
    if (order != nullptr) {
        _retrieveShipmentsFromOrder();
    }
}
//==========================================================
void Refund::reversePrices()
{
    m_shipping.reversePrice();
    for (auto it = m_articlesShipped.begin();
         it != m_articlesShipped.end(); ++it) {
        it.value()->reversePrice();
    }
}
//==========================================================
void Refund::_retrieveShipmentsFromOrder()
{
    m_shipments.clear();
    m_skuToArticleFromShipments.clear();
    auto allSkus = skusSet();
    for (auto shipment : m_order->getShipments()) {
        qint64 timeToRefundSec = shipment->getDateTime().secsTo(m_dateTime);
        //Q_ASSERT(timeToRefundSec > -86400);
        if (timeToRefundSec < 1 && timeToRefundSec > -86400) {
            // We do this so when vat is computed by date time, shipment are computed before refund
            m_dateTime = m_dateTime.addSecs(-timeToRefundSec + 1);
        }
        //Q_ASSERT(shipment->getDateTime() < getDateTime());
        auto shipmentSkus = shipment->skusSet();
        if (allSkus.intersects(shipmentSkus) || m_retrieveAllShipmentsOnInit) {
            m_shipments << shipment;
            for (auto article : m_articlesShipped) {
                for (auto articleShipment : shipment->getArticlesShipped()) {
                    QString sku = article->getSku();
                    if (articleShipment->getSku() == sku) {
                        m_skuToArticleFromShipments.insert(sku, articleShipment);
                    }
                }
            }
        }
    }
}
//==========================================================
QSharedPointer<Shipment> Refund::getFirstShipment(
        const QString &vatRegime, const QString &vatCountry) const
{
    if (m_order != nullptr) {
        for (auto shipment : m_order->getShipments()) {
            if (shipment->getRegimeVat() == vatRegime
                    && shipment->getCountryCodeVat() == vatCountry) {
                return shipment;
            }
        }
    }
    return QSharedPointer<Shipment>();
}
//==========================================================
void Refund::merge(const Refund &refund)
{
    Shipment::merge(*static_cast<const Shipment *>(&refund));
    if (m_addressTo.isEmpty()) {
        m_addressTo = refund.m_addressTo;
    }
    if (m_addressFrom.isEmpty()) {
        m_addressFrom = refund.m_addressFrom;
    }
    if (m_dateTime.isValid() && refund.m_dateTime.isValid() & m_dateTime > refund.m_dateTime) {
        m_dateTime = refund.m_dateTime;
    }
}
//==========================================================
Address Refund::addressTo() const
{
    return m_addressTo;
}
//==========================================================
Address Refund::addressFrom() const
{
    return m_order->getAddressTo();
}
//==========================================================
bool Refund::retrieveAllShipmentsOnInit() const
{
    return m_retrieveAllShipmentsOnInit;
}
//==========================================================
void Refund::setRetrieveAllShipmentsOnInit(
        bool newRetrieveAllShipmentsOnInit)
{
    m_retrieveAllShipmentsOnInit = newRetrieveAllShipmentsOnInit;
}
//==========================================================
bool Refund::wasGuessed() const
{
    return m_wasGuessed;
}
//==========================================================
void Refund::setWasGuessed(bool newWasGuessed)
{
    m_wasGuessed = newWasGuessed;
}
//==========================================================
void Refund::setAddressTo(const Address &addressTo)
{
    m_addressTo = addressTo;
}
//==========================================================
void Refund::computeVatRegime(
        double &, bool alsoRecomputeVat)
{
    computeVatRegime(alsoRecomputeVat);
}
//==========================================================
void Refund::computeVatRegime(bool alsoRecomputeVat)
{
    if (m_order != nullptr && m_order->getShipmentCount() > 0) {
        if (orderId() == "22082310203CDGH") {
            int TEMP=10;++TEMP;
        }
        //bool wasVatChared = qAbs(m_vatForRoundCorrection) > 0.001;
        auto pricesByVatRateBefore = getTotalPriceTaxesByVatRate();
        //_retrieveShipmentsFromOrder();
        QHash<QString, QHash<QString, double>> shippingRate;
        /// Get regime(s) from shipment
        for (auto shipment : m_shipments) {
            m_regimeVat = shipment->getRegimeVat();
            m_countryVat = shipment->getCountryCodeVat();
            m_countrySaleDeclaration = shipment->getCountrySaleDeclaration();
            shippingRate[m_regimeVat][m_countryVat]
                    = qMax(shippingRate[m_regimeVat][m_countryVat],
                           shipment->getShipping().vatRate());
        }
        /// Will start recomputing / computing
        //if (!hasSeveralVat()) {
        /// One regime / vat country only
        if (alsoRecomputeVat) {
            m_shipping.recomputeVat(shippingRate[m_regimeVat][m_countryVat]);
        }
        for (auto article : m_articlesShipped) {
            QString sku = article->getSku();
            if (m_skuToArticleFromShipments.contains(sku) && alsoRecomputeVat){
                //if (articlesBySkus[m_regimeVat][m_countryVat].contains(sku)) {
                double vatRate = m_skuToArticleFromShipments
                        .values(sku).first()->vatRate(); //TODO what if 2 same articles with 2 different VAT ?
                article->recomputeVat(vatRate);
            }
        }
        if (alsoRecomputeVat) {
            double taxes = getTotalPriceTaxes(false);
            if (qAbs(m_vatForRoundCorrection) > 0.001 && qAbs(taxes - m_vatForRoundCorrection) > 0.05) {
                m_vatForRoundCorrection = 0.;
                for (auto article : m_articlesShipped) {
                    m_vatForRoundCorrection += article->getTotalPriceTaxes();
                }
            }
        }
        double totalTaxes = getTotalPriceTaxes();
        if (qAbs(m_vatForRoundCorrection) > 0.005
                && qAbs(totalTaxes - m_vatForRoundCorrection) > 0.05) {
            double totalPriceTaxed = getTotalPriceTaxed();
            auto shipment = m_shipments.first();
            double totalTaxedShipment = shipment->getTotalPriceTaxed();
            double totalTaxesShipment = shipment->getTotalPriceTaxes();
            Q_ASSERT(false);
        }
    }
}
//==========================================================
QString Refund::channel() const
{
    return m_channel;
}
//==========================================================
void Refund::setChannel(const QString &channel)
{
    m_channel = channel;
}
//==========================================================
QString Refund::subchannel() const
{
    return m_subchannel;
}
//==========================================================
void Refund::setSubchannel(const QString &subchannel)
{
    m_subchannel = subchannel;
}
//==========================================================
bool Refund::isCompletelyLoaded() const
{
    if (m_order == nullptr) {
        return false;
    }
    // TODO with order id 112-0318778-8521823 from custom order
    // It shouldn't be shown as complete
    auto shipments = getShipments();
    for (auto shipment : getShipments()) {
        if (!shipment->isCompletelyLoaded()) {
            return false;
        }
    }
    //TODO launch exception about a refund with its order
    return shipments.size() > 0;
}
//==========================================================
QString Refund::orderId() const
{
    return m_orderId; //TODO
}
//==========================================================
Refund *Refund::fromString(const QString &string)
{
    QStringList values = string.split(SettingManager::SEP_TO_STRING_1);
    QHash<QString, QSharedPointer<ArticleSold>> articlesShipped;
    QStringList articleShippedElements
            = values[2].split(SettingManager::SEP_TO_STRING_LIST);
    for (auto keyValue = articleShippedElements.begin();
         keyValue != articleShippedElements.end(); ++keyValue) {
        QStringList elements = keyValue->split(
                    SettingManager::SEP_TO_STRING_MAP_KEY);
        articlesShipped[elements[0]]
                = QSharedPointer<ArticleSold>(
                    ArticleSold::fromString(elements[1],
                    SettingManager::SEP_TO_STRING_2));
    }
    Shipping shipping(values[3].toDouble()
            , values[4].toDouble()
            , values[5]
            );
    auto refund = new Refund(
                values[0]
            , values[1]
            , articlesShipped
            , shipping
            , QDateTime::fromString(values[6], SettingManager::DATE_FORMAT_ORDER)
            , Address::fromString(values[7], SettingManager::SEP_TO_STRING_2)
            , values[8]
            , values[9].toDouble()
            );
    refund->setChannel(values[10]);
    refund->setSubchannel(values[11]);
    refund->setInvoiceName(values[11]);
    return refund;
}
//==========================================================
QString Refund::toString() const
{
    QStringList values;
    values << m_id;
    values << m_orderId;
    QStringList articlesShippedString;
    QStringList articlesShippedStringValues;
    for (auto it=m_articlesShipped.begin();
         it!=m_articlesShipped.end(); ++it) {
        articlesShippedStringValues
                << it.key() + SettingManager::SEP_TO_STRING_MAP_KEY
                   + it.value()->toString(SettingManager::SEP_TO_STRING_2);
    }
    values << articlesShippedStringValues.join(SettingManager::SEP_TO_STRING_LIST);
    values << QString::number(m_shipping.totalPriceTaxed());
    values << QString::number(m_shipping.totalTaxes());
    values << m_shipping.currency();
    values << m_dateTime.toString(SettingManager::DATE_FORMAT_ORDER);
    values << m_addressTo.toString(SettingManager::SEP_TO_STRING_2);
    values << m_currency;
    values << QString::number(m_vatForRoundCorrection);
    values << m_channel;
    values << m_subchannel;
    values << m_invoiceNumber;
    return values.join(SettingManager::SEP_TO_STRING_1);
}
//==========================================================
QList<QSharedPointer<Shipment> > Refund::getShipments() const
{
    return m_shipments;
}
//==========================================================
