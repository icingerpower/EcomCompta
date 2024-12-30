#include "AddressesServiceCustomer.h"

//----------------------------------------------------------
AddressesServiceCustomer *AddressesServiceCustomer::instance()
{
    static AddressesServiceCustomer instance;
    return &instance;
}
//----------------------------------------------------------
AddressesServiceCustomer::AddressesServiceCustomer(
        QObject *parent)
{
    init("-self-invoices");
}
//----------------------------------------------------------
QString AddressesServiceCustomer::settingKeyPrefix() const
{
    return "-self-invoices";
}
//----------------------------------------------------------
