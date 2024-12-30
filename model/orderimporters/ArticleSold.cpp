#include "ArticleSold.h"
#include "ModelStockDeported.h"
#include "../common/currencies/CurrencyRateManager.h"
#include "Shipment.h"
#include "Order.h"
#include "model/bookkeeping/ManagerSaleTypes.h"
#include "model/CustomerManager.h"

//==========================================================
ArticleSold::ArticleSold()
{
    m_shipmentItemId = "";
    m_sku = "";
    m_name = "";
    m_saleType = "";
    m_units = -1;
    m_totalPriceTaxed = 0.;
    m_totalPriceTaxes = 0.;
}
//==========================================================
ArticleSold::ArticleSold(
        const QString &shipmentItemId,
        const QString &sku,
        const QString &name,
        const QString &saleType,
        int units,
        double totalPriceTaxed,
        double totalTaxes,
        const QString &currency)
{
    m_shipmentItemId = shipmentItemId;

    if (shipmentItemId == "Dn8pPl9vR") {
        int TEMP=10;++TEMP;
    }
    m_sku = sku;
    m_name = name;
    m_units = units;
    m_saleType = saleType;
    //Q_ASSERT(!m_saleType.contains("Dark"));
    Q_ASSERT(getSaleType() == ManagerSaleTypes::SALE_PRODUCTS
             || getSaleType() == ManagerSaleTypes::SALE_SERVICES
             || getSaleType() == ManagerSaleTypes::SALE_PAYMENT_FASCILITOR);
    m_shipment = nullptr;
    m_totalPriceTaxed = totalPriceTaxed;
    m_totalPriceTaxes = totalTaxes;
    m_currency = currency;
    ModelStockDeported::instance()->recordSku(sku, name, getUnitPriceUntaxed());
}
//==========================================================
void ArticleSold::init(const Shipment *shipment)
{
    m_shipment = shipment;
    m_shipping.init(shipment->getOrder());
}
//==========================================================
bool ArticleSold::operator==(const ArticleSold &other) const
{
    bool equals = m_sku == other.m_sku
            && m_shipmentItemId == other.m_shipmentItemId
            && m_name == other.m_name
            && m_saleType == other.m_saleType
            && m_units == other.m_units
            && qAbs(m_totalPriceTaxed - other.m_totalPriceTaxed) < 0.00001
            && qAbs(m_totalPriceTaxes - other.m_totalPriceTaxes) < 0.00001
            && m_shipping == other.m_shipping;
    return equals;
}
//==========================================================
void ArticleSold::merge(const ArticleSold &other)
{
    Q_ASSERT(other.m_shipmentItemId.isEmpty() || m_shipmentItemId == other.m_shipmentItemId
             && (other.m_sku.isEmpty() || m_sku == other.m_sku));
    if (m_name.isEmpty()) {
        m_name = other.m_name;
    }
    m_saleType = other.m_saleType;
    if (m_units == -1) {
        m_units = other.m_units;
    }
    if (m_totalPriceTaxed == 0.) {
        m_totalPriceTaxed = other.m_totalPriceTaxed;
    }
    if (m_totalPriceTaxes == 0.) {
        m_totalPriceTaxes = other.m_totalPriceTaxes;
    }
    if (m_shipment == nullptr) {
        init(other.m_shipment);
    }
    if (other.m_shipping.totalPriceTaxed() > m_shipping.totalPriceTaxed()) {
        m_shipping = other.m_shipping;
    }
}
//==========================================================
QString ArticleSold::getShipmentItemId() const
{
    return m_shipmentItemId;
}
//==========================================================
Shipping ArticleSold::getShipping() const
{
    return m_shipping;
}
//==========================================================
void ArticleSold::setShipping(const Shipping &shipping)
{
    m_shipping = shipping;
    Q_ASSERT(!m_shipping.currency().isNull());
}
//==========================================================
/*
QMap<QString, double> ArticleSold::getChargedFees() const
{
    return m_chargedFees;
}
//==========================================================
void ArticleSold::setChargedFees(
        const QMap<QString, double> &chargedFees)
{
    m_chargedFees = chargedFees;
}
//*/
//==========================================================
void ArticleSold::recomputeVat(double rate)
{
    double shippingTotal = m_shipping.totalPriceTaxed();
    double shippingTotalUntaxed = shippingTotal / (1. + rate);
    double shippingTaxes = shippingTotal - shippingTotalUntaxed;
    m_shipping.setTotalTaxes(shippingTaxes);
    double articleUntaxed = m_totalPriceTaxed / (1. + rate);
    m_totalPriceTaxes = m_totalPriceTaxed - articleUntaxed;
}
//==========================================================
double ArticleSold::vatRate() const
{
    double rate = 0;
    if (qAbs(m_totalPriceTaxes) > 0.001) {
        //rate = getTotalPriceTaxes() / getTotalPriceUntaxed();
        rate = m_totalPriceTaxes / (m_totalPriceTaxed-m_totalPriceTaxes);
    }
    return rate;
}
//==========================================================
QString ArticleSold::vatRateString() const
{
    return QString::number(vatRate(), 'f', 2);
}
//==========================================================
QString ArticleSold::toString(const QString &sep) const
{
    QStringList values;
    values << m_shipmentItemId;
    values << m_sku;
    values << m_name;
    values << m_saleType;
    values << QString::number(m_units);
    values << QString::number(m_totalPriceTaxed);
    values << QString::number(m_totalPriceTaxes);
    values << m_currency;
    values << QString::number(m_shipping.totalPriceTaxed());
    values << QString::number(m_shipping.totalTaxes());
    values << m_shipping.currency();
    return values.join(sep);
}
//==========================================================
ArticleSold *ArticleSold::fromString(
        const QString &string, const QString &sep)
{
    QStringList values
            = string.split(sep);
    if (values.size() == 10) {
        // TODO TEMP until all was saved properly
        values.insert(3, ManagerSaleTypes::SALE_PRODUCTS);
    }
    ArticleSold *articleSold = new ArticleSold(
                values[0]
                , values[1]
                , values[2]
                , values[3]
                , values[4].toInt()
                , values[5].toDouble()
                , values[6].toDouble()
                , values[7]
            );
    Q_ASSERT(articleSold->getSaleType() == ManagerSaleTypes::SALE_PRODUCTS
             || articleSold->getSaleType() == ManagerSaleTypes::SALE_SERVICES
             || articleSold->getSaleType() == ManagerSaleTypes::SALE_PAYMENT_FASCILITOR);
    Shipping shipping(values[8].toDouble()
            , values[9].toDouble()
            , values[10]);
    articleSold->setShipping(shipping);
    return articleSold;
}
//==========================================================
void ArticleSold::reversePrice()
{
    if (qAbs(m_totalPriceTaxed) > 0.001) {
        m_totalPriceTaxed = -m_totalPriceTaxed;
        if (qAbs(m_totalPriceTaxes) > 0.001) {
            m_totalPriceTaxes = -m_totalPriceTaxes;
        }
    }
    m_shipping.reversePrice();
}
//==========================================================
const QString &ArticleSold::getSku() const
{
    return m_sku;
}
//==========================================================
void ArticleSold::setSku(const QString &value)
{
    m_sku = value;
}
//==========================================================
QString ArticleSold::getName() const
{
    return m_name;
}
//==========================================================
void ArticleSold::setName(const QString &value)
{
    m_name = value;
}
//==========================================================
double ArticleSold::getTotalPriceTaxedConverted() const
{
    double converted =
            CurrencyRateManager::instance()->convert(
                getTotalPriceTaxed(),
                m_currency,
                CustomerManager::instance()->getSelectedCustomerCurrency(),
                m_shipment->getOrder()->getDateTime().date());
    return converted;
}
//==========================================================
double ArticleSold::getTotalPriceTaxed() const
{
    return m_totalPriceTaxed;
}
//==========================================================
void ArticleSold::setTotalPriceTaxed(double value)
{
    m_totalPriceTaxed = value;
}
//==========================================================
double ArticleSold::getTotalPriceTaxesConverted() const
{
    double converted =
            CurrencyRateManager::instance()->convert(
                getTotalPriceTaxes(),
                m_currency,
                CustomerManager::instance()->getSelectedCustomerCurrency(),
                m_shipment->getOrder()->getDateTime().date());
    return converted;
}
//==========================================================
double ArticleSold::getTotalPriceTaxes() const
{
    return m_totalPriceTaxes;
}
//==========================================================
void ArticleSold::setTotalPriceTaxes(double value)
{
    m_totalPriceTaxes = value;
}
//==========================================================
double ArticleSold::getTotalPriceUntaxedConverted() const
{
    double converted =
            CurrencyRateManager::instance()->convert(
                getTotalPriceUntaxed(),
                m_currency,
                CustomerManager::instance()->getSelectedCustomerCurrency(),
                m_shipment->getOrder()->getDateTime().date());
    return converted;
}
//==========================================================
double ArticleSold::getTotalPriceUntaxed() const
{
    double priceUntaxed = getTotalPriceTaxed() - getTotalPriceTaxes();
    return priceUntaxed;
}
//==========================================================
double ArticleSold::getUnitPriceTaxed() const
{
    double price = getTotalPriceTaxed() / m_units;
    return price;
}
//==========================================================
double ArticleSold::getUnitPriceTaxes() const
{
    double price = getTotalPriceTaxes() / m_units;
    return price;
}
//==========================================================
double ArticleSold::getUnitPriceUntaxed() const
{
    double price = getTotalPriceUntaxed() / m_units;
    return price;
}
//==========================================================
double ArticleSold::getUnitPriceTaxedConverted() const
{
    double converted =
            CurrencyRateManager::instance()->convert(
                getUnitPriceTaxed(),
                m_currency,
                CustomerManager::instance()->getSelectedCustomerCurrency(),
                m_shipment->getOrder()->getDateTime().date());
    return converted;
}
//==========================================================
double ArticleSold::getUnitPriceTaxesConverted() const
{
    double converted =
            CurrencyRateManager::instance()->convert(
                getUnitPriceTaxes(),
                m_currency,
                CustomerManager::instance()->getSelectedCustomerCurrency(),
                m_shipment->getOrder()->getDateTime().date());
    return converted;
}
//==========================================================
double ArticleSold::getUnitPriceUntaxedConvrted() const
{
    double converted =
            CurrencyRateManager::instance()->convert(
                getUnitPriceUntaxed(),
                m_currency,
                CustomerManager::instance()->getSelectedCustomerCurrency(),
                m_shipment->getOrder()->getDateTime().date());
    return converted;
}
//==========================================================
int ArticleSold::getUnits() const
{
    return m_units;
}
//==========================================================
void ArticleSold::setUnits(int value)
{
    m_units = value;
}
//==========================================================
QString ArticleSold::getSaleType() const
{
    return m_saleType;
}
//==========================================================
void ArticleSold::setSaleType(const QString &saleType)
{
    m_saleType = saleType;
}
//==========================================================
const Shipment *ArticleSold::getShipment() const
{
    return m_shipment;
}
//==========================================================
QString ArticleSold::getCurrency() const
{
    return m_currency;
}
//==========================================================
