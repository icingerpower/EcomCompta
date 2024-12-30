#ifndef ADDRESSESSERVICECUSTOMER_H
#define ADDRESSESSERVICECUSTOMER_H

#include "ShippingAddressesManager.h"

class AddressesServiceCustomer : public ShippingAddressesManager
{
    Q_OBJECT
public:
    static AddressesServiceCustomer *instance();

private:
    explicit AddressesServiceCustomer(
            QObject *parent = nullptr);
    virtual QString settingKeyPrefix() const;
};

#endif // ADDRESSESSERVICECUSTOMER_H
