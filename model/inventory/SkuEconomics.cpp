#include "../common/currencies/CurrencyRateManager.h"
#include "../common/countries/CountryManager.h"

#include "model/CustomerManager.h"
#include "model/vat/VatRateManager.h"

#include "SkuEconomics.h"

SkuEconomics::SkuEconomics()
{
    m_unitReturned = 0;
    m_unitSold = 0;
    m_unitPrice = 0;
}

void SkuEconomics::recordFee(
    const QString &fee,
    double count,
    double total,
    const QDate &date,
    const QString &currency)
{
    const auto &bookKeepingCur = CustomerManager::instance()->getSelectedCustomerCurrency();
    if (currency != bookKeepingCur)
    {
        double rate = CurrencyRateManager::instance()->rate(
                    currency, bookKeepingCur, date);
        total *= rate;
    }
    if (fee.contains("fulfilment", Qt::CaseInsensitive))
    {
        m_feesAmazon.insert(fee, Fee{count, total});
    }
    else if (fee.contains("storage", Qt::CaseInsensitive))
    {
        m_feesStorage.insert(fee, Fee{count, total});
    }
    else if (fee.contains("sponsored", Qt::CaseInsensitive))
    {
        m_feesAds.insert(fee, Fee{count, total});
    }
    else
    {
        m_feesOther.insert(fee, Fee{count, total});
    }
}

void SkuEconomics::recordAverageSalePriceTaxed(
    const QString &countryCode,
    const QString &sku,
    int unitSold,
    double averagePrice,
    const QDate &date,
    const QString &currency)
{
    if (averagePrice > 0.001)
    {
        const auto &bookKeepingCur
            = CustomerManager::instance()->getSelectedCustomerCurrency();
        if (currency != bookKeepingCur)
        {
            double rate = CurrencyRateManager::instance()->rate(
                currency, bookKeepingCur, date);
            averagePrice *= rate;
        }
        if (CountryManager::instance()->countriesCodeUE(date.year())
                ->contains(countryCode) || countryCode.toLower() == "uk")
        {
            double vatRate = VatRateManager::instance()->vatRate(
                countryCode, date, sku);
            averagePrice /= 1 + vatRate;
        }
        m_averageSalePrices.insert(unitSold, averagePrice);
    }
}

double SkuEconomics::averageSalePriceUntaxed() const
{
    double sum = 0.;
    int quantity = 0;
    for (auto it = m_averageSalePrices.begin();
         it != m_averageSalePrices.end(); ++it)
    {
        quantity += it.key();
        sum += it.key() * it.value();
    }
    if (quantity == 0)
    {
        return 0;
    }
    return sum / quantity;
}

void SkuEconomics::recordUnitSold(
    int unitSold, int unitReturned)
{
    m_unitSold += unitSold;
    m_unitReturned += unitReturned;
}

bool SkuEconomics::isUnitPriceRecorder() const
{
    return m_unitPrice > 0.001;
}

void SkuEconomics::recordUnitPrice(
    double price,
    int weightGrams,
    double shippingByKiloEUR,
    const QDate &date,
    const QString &currency)
{
    const auto &bookKeepingCur
        = CustomerManager::instance()->getSelectedCustomerCurrency();
    if (currency != bookKeepingCur)
    {
        double rate = CurrencyRateManager::instance()->rate(
                    currency, bookKeepingCur, date);
        price *= rate;
    }
    m_unitPrice = price + weightGrams * shippingByKiloEUR / 1000.;
}

double SkuEconomics::returnedRatio() const
{
    if (m_unitSold == 0)
    {
        return 0.;
    }
    return double(m_unitReturned) / m_unitSold;
}

double SkuEconomics::feesAmz() const
{
    double feesAmzQuantity = 0;
    double feesAmzTotal = 0.;
    for (auto it = m_feesAmazon.begin();
         it != m_feesAmazon.end(); ++it)
    {
        feesAmzQuantity += it.value().count;
        feesAmzTotal += it.value().total;
    }
    if (feesAmzQuantity == 0)
    {
        return 0.;
    }
    return feesAmzTotal / feesAmzQuantity;
}

double SkuEconomics::feesAds() const
{
    double feesAdsQuantity = 0;
    double feesAdsTotal = 0.;
    for (auto it = m_feesStorage.begin();
         it != m_feesStorage.end(); ++it)
    {
        feesAdsQuantity += it.value().count;
        feesAdsTotal += it.value().total;
    }
    if (feesAdsQuantity == 0)
    {
        return 0.;
    }
    return feesAdsTotal / feesAdsQuantity;
}

double SkuEconomics::feesStorage() const
{
    double feesStorageQuantity = 0;
    double feesStorageTotal = 0.;
    for (auto it = m_feesStorage.begin();
         it != m_feesStorage.end(); ++it)
    {
        feesStorageQuantity += it.value().count;
        feesStorageTotal += it.value().total;
    }
    if (feesStorageQuantity == 0)
    {
        return 0.;
    }
    return feesStorageTotal / feesStorageQuantity;
}

double SkuEconomics::profit() const
{
    if (m_averageSalePrices.size() == 0)
    {
        return 0.;
    }
    double profit = averageSalePriceUntaxed();
    profit -= feesAmz();
    profit -= m_unitPrice;
    return profit;
}

double SkuEconomics::profitWithStorage() const
{
    if (m_averageSalePrices.size() == 0)
    {
        return 0.;
    }
    double profitWithStorage = profit();
    profitWithStorage -= feesStorage();
    return profitWithStorage;
}

double SkuEconomics::profitWithAds() const
{
    if (m_averageSalePrices.size() == 0)
    {
        return 0.;
    }
    double profitWithAds = profitWithStorage();
    profitWithAds -= feesAds();
    return profitWithAds;
}
