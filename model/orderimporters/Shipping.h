#ifndef SHIPPING_H
#define SHIPPING_H

#include <QtCore/qstring.h>

class Order;

class Shipping
{
public:
    Shipping();
    Shipping(double totalPriceTaxed,
             double totalTaxes,
             const QString &currency);
    Shipping(const Shipping &shipping);
    void init(const Order *order);
    void addCost(const Shipping &shipping);
    bool operator==(const Shipping &other) const;
    //Shipping &operator=(const Shipping &other);
    bool isNull() const;
    void recomputeVat(double rate);
    double vatRate() const;
    QString vatRateString() const;
    void reversePrice(); /// Positive to negative, to create refund

    double totalPriceUntaxedConverted() const;
    double totalPriceTaxedConverted() const;
    double totalTaxesConverted() const;

    double totalPriceUntaxed() const;
    double totalPriceTaxed() const;
    void setTotalPriceTaxed(double totalPriceTaxed);

    double totalTaxes() const;
    void setTotalTaxes(double totalTaxes);

    QString currency() const;
    void setCurrency(const QString &currency);

private:
    const Order *m_order;
    QString m_currency;
    double m_totalPriceTaxed;
    double m_totalTaxes;
};

#endif // SHIPPING_H
