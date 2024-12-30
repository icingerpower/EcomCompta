#ifndef ARTICLE_H
#define ARTICLE_H

#include <QtCore/qstring.h>
#include <QtCore/qmap.h>

#include "model/SettingManager.h"

#include "Shipping.h"

class Shipment;

class ArticleSold
{
public:
    ArticleSold(const QString &shipmentItemId,
                const QString &sku,
                const QString &name,
                const QString &saleType,
                int units,
                double totalPriceTaxed,
                double totalTaxes,
                const QString &currency);
    void init(const Shipment *shipment);
    bool operator==(const ArticleSold &other) const;
    void merge(const ArticleSold &other);
    void recomputeVat(double rate);
    double vatRate() const;
    QString vatRateString() const;
    QString toString(const QString &sep = SettingManager::SEP_TO_STRING_2) const;
    static ArticleSold *fromString(
            const QString &string,
            const QString &sep = SettingManager::SEP_TO_STRING_2);
    void reversePrice(); /// Positive to negative, to create refund

    const QString &getSku() const;
    void setSku(const QString &value);

    QString getName() const;
    void setName(const QString &value);

    double getTotalPriceTaxedConverted() const;
    double getTotalPriceTaxed() const;
    void setTotalPriceTaxed(double value);

    double getTotalPriceTaxesConverted() const;
    double getTotalPriceTaxes() const;
    void setTotalPriceTaxes(double value);

    double getTotalPriceUntaxedConverted() const;
    double getTotalPriceUntaxed() const;

    double getUnitPriceTaxed() const;
    double getUnitPriceTaxes() const;
    double getUnitPriceUntaxed() const;
    double getUnitPriceTaxedConverted() const;
    double getUnitPriceTaxesConverted() const;
    double getUnitPriceUntaxedConvrted() const;

    int getUnits() const;
    void setUnits(int value);

    //QMap<QString, double> getChargedFees() const;
    //void setChargedFees(const QMap<QString, double> &chargedFees);

    Shipping getShipping() const;
    void setShipping(const Shipping &shipping);

    QString getShipmentItemId() const;


    QString getCurrency() const;

    const Shipment *getShipment() const;

    QString getSaleType() const;
    void setSaleType(const QString &saleType);

private:
    ArticleSold();
    QString m_sku;
    QString m_shipmentItemId;
    QString m_name;
    QString m_saleType;
    QString m_currency;
    int m_units;
    double m_totalPriceTaxed;
    double m_totalPriceTaxes;
    //QMap<QString, double> m_chargedFees;
    Shipping m_shipping;
    const Shipment *m_shipment;

};

#endif // ARTICLE_H
