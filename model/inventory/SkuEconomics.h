#ifndef SKUECONOMICS_H
#define SKUECONOMICS_H

#include <QString>
#include <QHash>
#include <QDate>

class SkuEconomics
{
public:
    SkuEconomics();
    double averageSalePriceUntaxed() const;
    bool isUnitPriceRecorder() const;
    double profitTotal() const;
    double quantitySold() const;
    double profit() const;
    double profitWithStorage() const;
    double profitWithAds() const;
    double returnedRatio() const;
    double feesAmzReferal() const;
    double feesAmz() const;
    double feesAds() const;
    double feesStorage() const;

    void recordFee(const QString &fee,
                   double count,
                   double total,
                   const QDate &date,
                   const QString &currency);
    void recordAverageSalePriceTaxed(const QString &countryCode,
                                     const QString &sku,
                                     int unitSold,
                                     double averagePrice,
                                     const QDate &date,
                                     const QString &currency);
    void recordUnitSold(int unitSold, int unitReturned);
    void recordUnitPrice(double price,
                         int weightGrams,
                         double shippingByKiloEUR,
                         const QDate &date,
                         const QString &currency);

    double unitPrice() const;

private:
    struct Fee{
        double count;
        double total;
    };
    QMultiHash<QString, Fee> m_feesAds;
    QMultiHash<QString, Fee> m_feesAmazon;
    QMultiHash<QString, Fee> m_feesReferal; // 15% amazon commission
    QMultiHash<QString, Fee> m_feesStorage;
    QMultiHash<QString, Fee> m_feesOther;
    QMultiHash<int, double> m_averageSalePrices;
    int m_unitSold;
    int m_unitReturned;
    double m_unitPrice;

};

#endif // SKUECONOMICS_H
