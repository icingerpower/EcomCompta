#include <qglobal.h>

#include "../common/currencies/CurrencyRateManager.h"

#include "Shipping.h"
#include "model/SettingManager.h"
#include "model/orderimporters/Order.h"
#include "model/CustomerManager.h"

//==========================================================
Shipping::Shipping()
{
    m_totalTaxes = 0.;
    m_totalPriceTaxed = 0.;
    m_order = nullptr;
}
//==========================================================
Shipping::Shipping(
        double totalPriceTaxed,
        double totalTaxes,
        const QString &currency)
{
    Q_ASSERT(!qIsNaN(totalTaxes));
    Q_ASSERT(!qIsNaN(totalPriceTaxed));
    m_totalTaxes = totalTaxes;
    m_totalPriceTaxed = totalPriceTaxed;
    m_currency = currency;
    //Q_ASSERT(!m_currency.isEmpty());
    m_order = nullptr;
}
//==========================================================
Shipping::Shipping(const Shipping &shipping)
    : Shipping(shipping.m_totalPriceTaxed,
               shipping.m_totalTaxes,
               shipping.m_currency)
{
    m_order = shipping.m_order;
}
//==========================================================
void Shipping::init(const Order *order)
{
    m_order = order;
}
//==========================================================
void Shipping::addCost(const Shipping &shipping)
{
    m_totalPriceTaxed += shipping.m_totalPriceTaxed;
    m_totalTaxes += shipping.m_totalTaxes;
}
//==========================================================
bool Shipping::operator==(const Shipping &other) const
{
    bool equals = qAbs(m_totalTaxes - other.m_totalTaxes) < 0.00001
            && qAbs(m_totalPriceTaxed - other.m_totalPriceTaxed) < 0.0001;
    return equals;
}
//==========================================================
/*
Shipping &Shipping::operator=(const Shipping &other)
{
    m_totalTaxes = other.m_totalTaxes;
    m_totalPriceTaxed = other.m_totalPriceTaxed;
    return *this;
}
//*/
//==========================================================
bool Shipping::isNull() const
{
    return m_currency.isEmpty() || qAbs(m_totalPriceTaxed) < 0.001;
}
//==========================================================
void Shipping::recomputeVat(double rate)
{
    double untaxed = m_totalPriceTaxed / (1. + rate);
    m_totalTaxes = m_totalPriceTaxed - untaxed;
}
//==========================================================
double Shipping::vatRate() const
{
    double rate = 0;
    if (qAbs(m_totalTaxes) > 0) {
        rate = m_totalTaxes / totalPriceUntaxed();
    }
    return rate;
}
//==========================================================
QString Shipping::vatRateString() const
{
    return QString::number(vatRate(), 'f', 2);
}
//==========================================================
void Shipping::reversePrice()
{
    if (qAbs(m_totalPriceTaxed) > 0.001) {
        m_totalPriceTaxed = -m_totalPriceTaxed;
        if (qAbs(m_totalTaxes) > 0.001) {
            m_totalTaxes = -m_totalTaxes;
        }
    }
}
//==========================================================
double Shipping::totalPriceUntaxedConverted() const
{
    if (isNull()) {
        return 0.;
    }
    double converted =
            CurrencyRateManager::instance()->convert(
                totalPriceUntaxed(),
                m_currency,
                CustomerManager::instance()->getSelectedCustomerCurrency(),
                m_order->getDateTime().date());
    return converted;
}
//==========================================================
double Shipping::totalPriceTaxedConverted() const
{
    if (isNull()) {
        return 0.;
    }
    double converted =
            CurrencyRateManager::instance()->convert(
                totalPriceTaxed(),
                m_currency,
                CustomerManager::instance()->getSelectedCustomerCurrency(),
                m_order->getDateTime().date());
    return converted;
}
//==========================================================
double Shipping::totalTaxesConverted() const
{
    if (isNull()) {
        return 0.;
    }
    double converted =
            CurrencyRateManager::instance()->convert(
                totalTaxes(),
                m_currency,
                CustomerManager::instance()->getSelectedCustomerCurrency(),
                m_order->getDateTime().date());
    return converted;
}
//==========================================================
double Shipping::totalPriceUntaxed() const
{
    return totalPriceTaxed() - totalTaxes();
}
//==========================================================
double Shipping::totalPriceTaxed() const
{
    return m_totalPriceTaxed;
}
//==========================================================
void Shipping::setTotalPriceTaxed(double totalPriceTaxed)
{
    m_totalPriceTaxed = totalPriceTaxed;
}
//==========================================================
double Shipping::totalTaxes() const
{
    return m_totalTaxes;
}
//==========================================================
void Shipping::setTotalTaxes(double totalTaxes)
{
    m_totalTaxes = totalTaxes;
}
//==========================================================
QString Shipping::currency() const
{
    return m_currency;
}
//==========================================================
void Shipping::setCurrency(const QString &currency)
{
    m_currency = currency;
}
//==========================================================
